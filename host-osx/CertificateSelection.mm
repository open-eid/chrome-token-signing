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

@interface CertificateSelection () <NSTableViewDataSource,NSTableViewDelegate> {
    IBOutlet NSPanel *certificateSelectionPanel;
    IBOutlet NSTableView *certificateSelection;
    IBOutlet NSButton *okButton;
    IBOutlet NSButton *cancelButton;
    IBOutlet NSTextField *warningLabel;
    NSMutableArray *certificates;
}

@end

@implementation CertificateSelection

+ (NSDictionary *)show
{
    CertificateSelection *dialog = [[CertificateSelection alloc] init];
    dialog->certificates = [[NSMutableArray alloc] init];

    PKCS11CardManager manager(PKCS11_MODULE);
    time_t currentTime = DateUtils::now();
    for (auto &token : manager.getAvailableTokens()) {
        CardManager *local = manager.getManagerForReader(token);

        if (local->isCardInReader()) {
            time_t validTo = local->getValidTo();
            if (currentTime > validTo)
                continue;

            std::vector<unsigned char> cert = local->getSignCert();
            [dialog->certificates addObject: @{
                @"cert": @(BinaryUtils::bin2hex(cert).c_str()),
                @"validFrom": @(DateUtils::timeToString(local->getValidFrom()).c_str()),
                @"validTo": @(DateUtils::timeToString(validTo).c_str()),
                @"CN": @(local->getCN().c_str()),
                @"type": @(local->getType().c_str()),
            }];
        }
        delete local;
    }

    if (dialog->certificates.count == 0) {
        return @{@"result": @"no_certificates"};
    }

    [NSBundle loadNibNamed:@"CertificateSelection" owner:dialog];
    [dialog->certificateSelection setDoubleAction:@selector(okClicked:)];
    if (dialog->certificateSelection.numberOfRows > 0) {
        [dialog->certificateSelection selectRowIndexes:[NSIndexSet indexSetWithIndex:0] byExtendingSelection:FALSE];
        dialog->okButton.enabled = YES;
    }
    [NSApp activateIgnoringOtherApps:YES];
    if ([NSApp runModalForWindow:dialog->certificateSelectionPanel] == NSModalResponseAbort ||
        dialog->certificateSelection.selectedRow == -1) {
        return @{@"result": @"user_cancel"};
    }
    return @{@"result": @"ok", @"cert": dialog->certificates[dialog->certificateSelection.selectedRow][@"cert"]};
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
