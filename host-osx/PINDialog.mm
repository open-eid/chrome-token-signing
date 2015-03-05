/* Chrome Linux plugin
 *
 * This software is released under either the GNU Library General Public
 * License (see LICENSE.LGPL).
 *
 * Note that the only valid version of the LGPL license as far as this
 * project is concerned is the original GNU Library General Public License
 * Version 2.1, February 1999
 */

#import "PINDialog.h"

#import "../host-shared/PKCS11CardManager.h"

#include <future>

@interface PINPanel () {
    IBOutlet NSPanel *pinPanel;
    IBOutlet NSButton *okButton;
    IBOutlet NSButton *cancelButton;
    IBOutlet NSTextField *nameLabel;
    IBOutlet NSTextField *messageField;
    IBOutlet NSSecureTextField *pinField;
    IBOutlet NSTextField *pinFieldLabel;
    IBOutlet NSProgressIndicator *progressBar;
}

@end

@implementation PINPanel

+ (NSDictionary *)show:(NSDictionary*)params
{
    if (!params[@"hash"] || [params[@"hash"] length] % 2 == 1) {
        return @{@"result": @"invalid_argument"};
    }
    std::vector<unsigned char> hash = BinaryUtils::hex2bin([params[@"hash"] UTF8String]);
    switch (hash.size()) {
        case 20: // SHA1
        case 28: // SHA224
        case 32: // SHA256
        case 48: // SHA384
        case 64: break; // SHA512
        default: return @{@"result": @"invalid_argument"};
    }

    PKCS11CardManager manager(PKCS11_MODULE);
    time_t currentTime = DateUtils::now();
    std::unique_ptr<PKCS11CardManager> selected;
    for (auto &token : manager.getAvailableTokens()) {
        selected.reset(manager.getManagerForReader(token));
        if (!selected->isCardInReader()) {
            selected.reset();
            continue;
        }

        std::vector<unsigned char> cert = selected->getSignCert();
        if (BinaryUtils::hex2bin([params[@"cert"] UTF8String]) != cert || currentTime > selected->getValidTo()) {
            selected.reset();
            continue;
        }

        break;
    }

    if (!selected) {
        return @{@"result": @"invalid_argument"};
    }

    int retriesLeft = selected->getPIN2RetryCount();
    if (retriesLeft == 0) {
        return @{@"result": @"pin_blocked"};
    }

    std::vector<unsigned char> signature;
    std::future<void> future;
    PINPanel *dialog = [[PINPanel alloc] init];
    NSTimer *timer;
    if (selected->isPinpad()) {
        [NSBundle loadNibNamed:@"PINPadDialog" owner:dialog];
        [dialog->progressBar setDoubleValue:30];
        [dialog->progressBar startAnimation:self];
        timer = [NSTimer scheduledTimerWithTimeInterval:1.0 target:dialog selector:@selector(handleTimerTick:) userInfo:nil repeats:YES];
        [[NSRunLoop currentRunLoop] addTimer:timer forMode:NSModalPanelRunLoopMode];
        future = std::async(std::launch::async, [&](){
            try {
                signature = selected->sign(hash, PinString());
            }
            catch(const AuthenticationErrorAborted &) {
                [NSApp abortModal];
                return;
            }
            catch(const AuthenticationError &) {
                [NSApp abortModal];
                return;
            }
            catch(const std::runtime_error &) {
                [NSApp abortModal];
                return;
            }
            [NSApp stopModal];
        });
    }
    else {
        [NSBundle loadNibNamed:@"PINDialog" owner:dialog];
    }
    [dialog->nameLabel setTitleWithMnemonic:@((selected->getCardName() + ", " + selected->getPersonalCode()).c_str())];
    if (retriesLeft < 3) {
        [dialog->messageField setTitleWithMnemonic:@(("Tries left: " + std::to_string(retriesLeft)).c_str())];
    }
    [NSApp activateIgnoringOtherApps:YES];
    NSModalResponse result = [NSApp runModalForWindow:dialog->pinPanel];

    if (timer) {
        [timer invalidate];
        timer = nil;
    }

    if (result == NSModalResponseAbort) {
        return @{@"result": @"user_cancel"};
    }

    if (!selected->isPinpad()) {
        signature = selected->sign(
            BinaryUtils::hex2bin([params[@"hash"] UTF8String]),
            PinString(dialog->pinField.stringValue.UTF8String));
    }

    return @{@"signature":@(BinaryUtils::bin2hex(signature).c_str())};
}

- (IBAction)okClicked:(id)sender
{
    [NSApp stopModal];
}

- (IBAction)cancelClicked:(id)sender
{
    [NSApp abortModal];
}

- (void)controlTextDidChange:(NSNotification*)notification;
{
    [okButton setEnabled:(pinField.stringValue.length >= 5)];
}

- (void)handleTimerTick:(NSTimer*)timer
{
    [progressBar incrementBy:-1];
    if (progressBar.doubleValue <= 0) {
        [progressBar stopAnimation:self];
        [NSApp abortModal];
    }
}

@end