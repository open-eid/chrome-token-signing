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

- (bool)isDuplicate:(NSString*) certificate {
    for(NSUInteger i = 0; i < [certificates count]; i++) {
        if ([certificates[i][@"cert"] isEqualToString:certificate]) return true;
    }
    return false;
}

- (instancetype)init
{
    if (self = [super init]) {
        certificates = [[NSMutableArray alloc] init];
        try {
            NSDateFormatter *df = [[NSDateFormatter alloc] init];
            df.dateFormat = @"dd.MM.YYYY";
            NSDateFormatter *asn1 = [[NSDateFormatter alloc] init];
            asn1.dateFormat = @"yyyyMMddHHmmss'Z'";
            asn1.timeZone = [NSTimeZone timeZoneForSecondsFromGMT:0];
            std::string pkcs11ModulePath(PKCS11Path::getPkcs11ModulePath());
            for (auto &token : PKCS11CardManager::instance(pkcs11ModulePath)->getAvailableTokens()) {
                PKCS11CardManager *local = PKCS11CardManager::instance(pkcs11ModulePath)->getManagerForReader(token);
                if (!local -> hasSignCert()) {
                    _log("no signing certificate, moving on to next token...");
                    delete local;
                    continue;
                }
                NSDate *date = [asn1 dateFromString:@(local->getValidTo().c_str())];
                if ([date compare:NSDate.date] > 0 && ![self isDuplicate:@(BinaryUtils::bin2hex(local->getSignCert()).c_str())]) {
                    _log("token has valid signing certificate, adding it to selection");
                    [certificates addObject: @{
                        @"cert": @(BinaryUtils::bin2hex(local->getSignCert()).c_str()),
                        @"validTo": [df stringFromDate:date],
                        @"CN": @(local->getCN().c_str()),
                        @"type": @(local->getType().c_str()),
                    }];
                }
                delete local;
            }
        } catch (const std::runtime_error &e) {
            self = nil;
            return self;
        }

        if (![NSBundle loadNibNamed:@"CertificateSelection" owner:self]) {
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

+ (NSDictionary *)show
{
    CertificateSelection *dialog = [[CertificateSelection alloc] init];
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
    if ([[tableColumn identifier] isEqualToString:@"CN"]) {
        return certificates[row][@"CN"];
    }
    if ([[tableColumn identifier] isEqualToString:@"type"]) {
        return certificates[row][@"type"];
    }
    if ([[tableColumn identifier] isEqualToString:@"validTo"]) {
        return certificates[row][@"validTo"];
    }
    return [NSString string];
}

#pragma mark - NSTableViewDelegate

- (BOOL)tableView:(NSTableView*)tableView shouldSelectRow:(NSInteger)row
{
    return okButton.enabled = YES;
}

@end
