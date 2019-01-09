/*
 * Chrome Token Signing Native Host
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#import "CertificateSelection.h"

#import "BinaryUtils.h"
#import "PKCS11CardManager.h"
#import "Labels.h"
#import "PKCS11Path.h"
#import "Logger.h"

#import <SecurityInterface/SFCertificatePanel.h>
#import <SecurityInterface/SFCertificateView.h>
#import <Security/Security.h>

#define _L(KEY) @(Labels::l10n.get(KEY).c_str())

static NSTouchBarItemIdentifier touchBarItemGroupId = @"ee.ria.chrome-token-signing.touchbar.group";
static NSTouchBarItemIdentifier touchBarItemSelectId = @"ee.ria.chrome-token-signing.touchbar.select";
static NSTouchBarItemIdentifier touchBarItemCancelId = @"ee.ria.chrome-token-signing.touchbar.cancel";
static NSTouchBarItemIdentifier touchBarItemSegmentId = @"ee.ria.chrome-token-signing.touchbar.segment";

@interface CertificateSelection () <NSTableViewDataSource,NSTableViewDelegate,NSTouchBarDelegate> {
    IBOutlet NSPanel *window;
    IBOutlet NSTableView *certificateSelection;
    IBOutlet NSButton *okButton;
    IBOutlet NSButton *cancelButton;
    IBOutlet NSTextField *warningLabel;
    NSMutableArray *certificates;
}

@end

@implementation CertificateSelection

- (bool)isDuplicate:(NSString*)certificate {
    for (NSDictionary *cert in certificates) {
        if ([cert[@"cert"] isEqualToString:certificate])
            return true;
    }
    return false;
}

- (instancetype)init:(bool)forSigning
{
    if (self = [super init]) {
        certificates = [[NSMutableArray alloc] init];
        NSDateFormatter *df = [[NSDateFormatter alloc] init];
        df.dateFormat = @"dd.MM.YYYY";
        NSDateComponents *components = [[NSDateComponents alloc] init];
        components.year = 2001;
        PKCS11Path::Params p11 = PKCS11Path::getPkcs11ModulePath();
        for (const PKCS11CardManager::Token &token : PKCS11CardManager(p11.path).tokens()) {
            CFDataRef data = CFDataCreateWithBytesNoCopy(nil, token.cert.data(), token.cert.size(), kCFAllocatorNull);
            SecCertificateRef cert = SecCertificateCreateWithData(nil, data);
            CFRelease(data);
            NSDictionary *dict = CFBridgingRelease(SecCertificateCopyValues(cert, nil, nil));
            CFRelease(cert);

            NSNumber *ku = dict[(__bridge id)kSecOIDKeyUsage][(__bridge id)kSecPropertyKeyValue];
            const bool isNonRepudiation = ku.unsignedIntValue & kSecKeyUsageNonRepudiation;
            if (forSigning != isNonRepudiation) {
                _log("certificate is non-repu: %u, requesting signing certificate %u, moving on to next token...", isNonRepudiation, forSigning);
                continue;
            }

            NSNumber *na = dict[(__bridge id)kSecOIDX509V1ValidityNotAfter][(__bridge id)kSecPropertyKeyValue];
            NSDate *date = [NSDate dateWithTimeInterval:na.intValue sinceDate:[NSCalendar.currentCalendar dateFromComponents:components]];
            NSString *hex = @(BinaryUtils::bin2hex(token.cert).c_str());
            if ([date compare:NSDate.date] <= 0 || [self isDuplicate:hex]) {
                _log("token has expired or is duplicate");
                continue;
            }

            _log("token has valid signing certificate, adding it to selection");
            NSString *cn = [NSString string];
            NSString *type = [NSString string];
            for (NSDictionary *item in dict[(__bridge id)kSecOIDX509V1SubjectName][(__bridge id)kSecPropertyKeyValue]) {
                if ([item[(__bridge id)kSecPropertyKeyLabel] isEqualToString:(__bridge id)kSecOIDCommonName]) {
                    cn = item[(__bridge id)kSecPropertyKeyValue];
                }
                if ([item[(__bridge id)kSecPropertyKeyLabel] isEqualToString:(__bridge id)kSecOIDOrganizationName]) {
                    type = item[(__bridge id)kSecPropertyKeyValue];
                }
            }

            if (type.length == 0) {
                for (NSDictionary *item in dict[(__bridge id)kSecOIDCertificatePolicies][(__bridge id)kSecPropertyKeyValue]) {
                    if (![item[(__bridge id)kSecPropertyKeyValue] isKindOfClass:[NSString class]]) {
                        continue;
                    }
                    NSString *value = item[(__bridge id)kSecPropertyKeyValue];
                    if ([value isEqual:@"1.3.6.1.4.1.51361.1.1.3"] || [value isEqual:@"1.3.6.1.4.1.51361.1.2.3"]) {
                        type = @"ESTEID (DIGI-ID)";
                    }
                    else if ([value isEqual:@"1.3.6.1.4.1.51361.1.1.4"] || [value isEqual:@"1.3.6.1.4.1.51361.1.2.4"]) {
                        type = @"ESTEID (DIGI-ID E-RESIDENT)";
                    }
                    else if ([value hasPrefix:@"1.3.6.1.4.1.51361.1"] || [value hasPrefix:@"1.3.6.1.4.1.51455.1"]) {
                        type = @"ESTEID";
                    }
                }
            }

            [certificates addObject: @{@"cert": hex, @"validTo": [df stringFromDate:date], @"CN": cn, @"type": type}];
        }

        if (![[NSBundle bundleForClass:CertificateSelection.class] loadNibNamed:@"CertificateSelection" owner:self topLevelObjects:nil]) {
            self = nil;
            return self;
        }

        if (@available(macOS 10_12_2, *)) {
            window.touchBar = [self makeTouchBar];
        }
        window.title = _L("select certificate");
        cancelButton.title = _L("cancel");
        okButton.title = _L("select");
        warningLabel.stringValue = _L("cert info");
        [[certificateSelection tableColumnWithIdentifier:@"CN"].headerCell setStringValue:_L("certificate")];
        [[certificateSelection tableColumnWithIdentifier:@"type"].headerCell setStringValue:_L("type")];
        [[certificateSelection tableColumnWithIdentifier:@"validTo"].headerCell setStringValue:_L("valid to")];
        if (certificateSelection.numberOfRows > 0) {
            [certificateSelection selectRowIndexes:[NSIndexSet indexSetWithIndex:0] byExtendingSelection:FALSE];
            okButton.enabled = YES;
        }
    }
    return self;
}

+ (NSDictionary *)show:(bool)forSigning
{
    try {
        CertificateSelection *dialog = [[CertificateSelection alloc] init:forSigning];
        if (!dialog) {
            return @{@"result": @"technical_error"};
        }
        if (dialog->certificates.count == 0) {
            return @{@"result": @"no_certificates"};
        }
        [NSApp activateIgnoringOtherApps:YES];
        NSModalResponse result = [NSApp runModalForWindow:dialog->window];
        [dialog->window close];
        if (result == NSModalResponseAbort || dialog->certificateSelection.selectedRow == -1) {
            return @{@"result": @"user_cancel"};
        }
        return @{@"cert": dialog->certificates[dialog->certificateSelection.selectedRow][@"cert"]};
    } catch(const BaseException &e) {
        _log("Exception: %s", e.what());
        return @{@"result": @(e.getErrorCode().c_str())};
    }
}

- (void)showCertificate
{
    NSString *hex = certificates[certificateSelection.selectedRow][@"cert"];
    std::vector<unsigned char> der = BinaryUtils::hex2bin(hex.UTF8String);
    CFDataRef data = CFDataCreateWithBytesNoCopy(nil, der.data(), der.size(), kCFAllocatorNull);
    id cert = CFBridgingRelease(SecCertificateCreateWithData(nil, data));
    CFRelease(data);
    SFCertificatePanel *panel = [[SFCertificatePanel alloc] init];
    [panel beginSheetForWindow:window modalDelegate:nil didEndSelector:nil contextInfo:nil certificates:@[cert] showGroup:NO];
}

- (IBAction)okClicked:(id)sender
{
    [NSApp stopModal];
}

- (IBAction)cancelClicked:(id)sender
{
    [NSApp abortModal];
}

- (IBAction)enableOkButton:(id)sender
{
    okButton.enabled = certificateSelection.selectedRow != -1;
}

- (IBAction)changeSelection:(id)sender
{
    NSUInteger index = certificateSelection.selectedRow >= 0 ? certificateSelection.selectedRow : 0;
    switch ([sender selectedSegment]) {
        case 0:
            if (index > 0)
                [certificateSelection selectRowIndexes:[NSIndexSet indexSetWithIndex:--index] byExtendingSelection:NO];
            break;
        case 1:
            if (index < certificates.count -1)
                [certificateSelection selectRowIndexes:[NSIndexSet indexSetWithIndex:++index] byExtendingSelection:NO];
            break;
        default:
            [self showCertificate];
            break;
    }
}

#pragma mark - NSTouchBarProvider

- (NSTouchBar *)makeTouchBar NS_AVAILABLE_MAC(10_12_2)
{
    NSTouchBar *touchBar = [[NSTouchBar alloc] init];
    touchBar.delegate = self;
    touchBar.defaultItemIdentifiers = @[NSTouchBarItemIdentifierFlexibleSpace, touchBarItemSegmentId, touchBarItemGroupId];
    touchBar.principalItemIdentifier = touchBarItemGroupId;
    return touchBar;
}

#pragma mark - NSTouchBarDelegate

- (NSTouchBarItem *)touchBar:(NSTouchBar *)touchBar makeItemForIdentifier:(NSTouchBarItemIdentifier)identifier NS_AVAILABLE_MAC(10_12_2)
{
    if ([identifier isEqualToString:touchBarItemGroupId])
    {
        NSArray *items = @[[self touchBar:touchBar makeItemForIdentifier:touchBarItemCancelId],
                           [self touchBar:touchBar makeItemForIdentifier:touchBarItemSelectId]];
        return [NSGroupTouchBarItem groupItemWithIdentifier:identifier items:items];
    }
    if ([identifier isEqualToString:touchBarItemSegmentId])
    {
        NSSegmentedControl *control = [NSSegmentedControl segmentedControlWithImages:
            @[[NSImage imageNamed:NSImageNameTouchBarGoUpTemplate], [NSImage imageNamed:NSImageNameTouchBarGoDownTemplate], [NSImage imageNamed:NSImageNameTouchBarGetInfoTemplate]]
            trackingMode:NSSegmentSwitchTrackingMomentary target:self action:@selector(changeSelection:)];
        NSCustomTouchBarItem *touchBarItem = [[NSCustomTouchBarItem alloc] initWithIdentifier:identifier];
        touchBarItem.view = control;
        return touchBarItem;
    }
    if ([identifier isEqualToString:touchBarItemSelectId])
    {
        NSButton *button = [NSButton buttonWithTitle:_L("select") target:self action:@selector(okClicked:)];
        button.bezelColor = NSColor.selectedMenuItemColor;
        NSCustomTouchBarItem *touchBarItem = [[NSCustomTouchBarItem alloc] initWithIdentifier:identifier];
        touchBarItem.view = button;
        return touchBarItem;
    }
    if ([identifier isEqualToString:touchBarItemCancelId])
    {
        NSCustomTouchBarItem *touchBarItem = [[NSCustomTouchBarItem alloc] initWithIdentifier:identifier];
        touchBarItem.view = [NSButton buttonWithTitle:_L("cancel") target:self action:@selector(cancelClicked:)];
        return touchBarItem;
    }
    return nil;
}

#pragma mark - NSTableViewDataSource

- (NSInteger)numberOfRowsInTableView:(NSTableView*)tableView
{
    return certificates.count;
}

- (id)tableView:(NSTableView*)tableView objectValueForTableColumn:(NSTableColumn*)tableColumn row:(NSInteger)row
{
    if (certificates.count == 0) {
        return [NSString string];
    }
    if ([tableColumn.identifier isEqualToString:@"CN"]) {
        return certificates[row][@"CN"];
    }
    if ([tableColumn.identifier isEqualToString:@"type"]) {
        return certificates[row][@"type"];
    }
    if ([tableColumn.identifier isEqualToString:@"validTo"]) {
        return certificates[row][@"validTo"];
    }
    return [NSString string];
}

#pragma mark - NSTableViewDelegate

- (BOOL)tableView:(NSTableView*)tableView shouldSelectRow:(NSInteger)row
{
    return okButton.enabled = YES;
}

- (BOOL)tableView:(NSTableView*)tableView shouldTypeSelectForEvent:(NSEvent*)event withCurrentSearchString:(NSString*)searchString
{
    switch ([event.charactersIgnoringModifiers characterAtIndex:0]) {
        case 0x20:
        case 0x49:
            [self showCertificate];
            return NO;
        default: return YES;
    }
}

@end
