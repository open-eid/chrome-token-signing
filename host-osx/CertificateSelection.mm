/* Chrome Linux plugin
*
* This software is released under either the GNU Library General Public
* License (see LICENSE.LGPL).
*
* Note that the only valid version of the LGPL license as far as this
* project is concerned is the original GNU Library General Public License
* Version 2.1, February 1999
*/

#import "CertificateSelection.h"

#import "../host-shared/PKCS11CardManager.h"
#import "../host-shared/BinaryUtils.h"

#define _L(KEY) @(l10nLabels.get(KEY).c_str())

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

- (instancetype)init
{
    if (self = [super init]) {
        certificates = [[NSMutableArray alloc] init];
        try {
            PKCS11CardManager manager;
            time_t currentTime = DateUtils::now();
            for (auto &token : manager.getAvailableTokens()) {
                PKCS11CardManager *local = manager.getManagerForReader(token);
                time_t validTo = local->getValidTo();
                if (currentTime <= validTo) {
                    std::vector<unsigned char> cert = local->getSignCert();
                    [certificates addObject: @{
                                               @"cert": @(BinaryUtils::bin2hex(cert).c_str()),
                                               @"validTo": @(DateUtils::timeToString(validTo).c_str()),
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
        [warningLabel setTitleWithMnemonic:_L("cert info")];
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
