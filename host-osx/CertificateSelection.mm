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

@interface CertificateSelection () <NSTableViewDataSource,NSTableViewDelegate> {
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
        try {
            NSDateFormatter *df = [[NSDateFormatter alloc] init];
            df.dateFormat = @"dd.MM.YYYY";
            PKCS11Path::Params p11 = PKCS11Path::getPkcs11ModulePath();
            for (const PKCS11CardManager::Token &token : PKCS11CardManager(p11.path).tokens()) {
                CFDataRef data = CFDataCreateWithBytesNoCopy(nil, token.cert.data(), token.cert.size(), kCFAllocatorNull);
                SecCertificateRef cert = SecCertificateCreateWithData(nil, data);
                CFRelease(data);
                NSDictionary *dict = CFBridgingRelease(SecCertificateCopyValues(cert, nil, nil));
                CFRelease(cert);

                NSNumber *ku = dict[(__bridge NSString*)kSecOIDKeyUsage][(__bridge NSString*)kSecPropertyKeyValue];
                const bool isNonRepudiation = ku.unsignedIntValue & kSecKeyUsageNonRepudiation;
                if (forSigning != isNonRepudiation) {
                    _log("certificate is non-repu: %u, requesting signing certificate %u, moving on to next token...", isNonRepudiation, forSigning);
                    continue;
                }

                NSDateComponents *components = [[NSDateComponents alloc] init];
                components.year = 2001;
                NSNumber *na = dict[(__bridge NSString*)kSecOIDX509V1ValidityNotAfter][(__bridge NSString*)kSecPropertyKeyValue];
                NSDate *date = [NSDate dateWithTimeInterval:na.intValue sinceDate:[NSCalendar.currentCalendar dateFromComponents:components]];
                NSString *hex = @(BinaryUtils::bin2hex(token.cert).c_str());
                if ([date compare:NSDate.date] <= 0 || [self isDuplicate:hex]) {
                    _log("token has expired or is duplicate");
                    continue;
                }

                _log("token has valid signing certificate, adding it to selection");
                NSString *cn = [NSString string];
                NSString *type = [NSString string];
                for (NSDictionary *item in dict[(__bridge NSString*)kSecOIDX509V1SubjectName][(__bridge NSString*)kSecPropertyKeyValue]) {
                    if ([item[(__bridge NSString*)kSecPropertyKeyLabel] isEqualToString:(__bridge NSString*)kSecOIDCommonName]) {
                        cn = item[(__bridge NSString*)kSecPropertyKeyValue];
                    }
                    if ([item[(__bridge NSString*)kSecPropertyKeyLabel] isEqualToString:(__bridge NSString*)kSecOIDOrganizationName]) {
                        type = item[(__bridge NSString*)kSecPropertyKeyValue];
                    }
                }

                [certificates addObject: @{@"cert": hex, @"validTo": [df stringFromDate:date], @"CN": cn, @"type": type}];
            }
        } catch (const std::runtime_error &e) {
            self = nil;
            return self;
        }

        if (![NSBundle.mainBundle loadNibNamed:@"CertificateSelection" owner:self topLevelObjects:nil]) {
            self = nil;
            return self;
        }

        window.title = _L("select certificate");
        cancelButton.title = _L("cancel");
        okButton.title = _L("select");
        warningLabel.stringValue = _L("cert info");
        [[certificateSelection tableColumnWithIdentifier:@"CN"].headerCell setStringValue:_L("certificate")];
        [[certificateSelection tableColumnWithIdentifier:@"type"].headerCell setStringValue:_L("type")];
        [[certificateSelection tableColumnWithIdentifier:@"validTo"].headerCell setStringValue:_L("valid to")];
        [certificateSelection setDoubleAction:@selector(okClicked:)];
        if (certificateSelection.numberOfRows > 0) {
            [certificateSelection selectRowIndexes:[NSIndexSet indexSetWithIndex:0] byExtendingSelection:FALSE];
            okButton.enabled = YES;
        }
    }
    return self;
}

+ (NSDictionary *)show:(bool)forSigning
{
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
        {
            NSString *hex = certificates[certificateSelection.selectedRow][@"cert"];
            std::vector<unsigned char> der = BinaryUtils::hex2bin(hex.UTF8String);
            CFDataRef data = CFDataCreateWithBytesNoCopy(nil, der.data(), der.size(), kCFAllocatorNull);
            id cert = CFBridgingRelease(SecCertificateCreateWithData(nil, data));
            CFRelease(data);
            SFCertificatePanel *panel = [[SFCertificatePanel alloc] init];
            [panel beginSheetForWindow:window modalDelegate:nil didEndSelector:nil contextInfo:nil certificates:@[cert] showGroup:NO];
            return NO;
        }
        default: return YES;
    }
}

@end
