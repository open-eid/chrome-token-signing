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
#import "../host-shared/BinaryUtils.h"

#include <future>

#define _L(KEY) @(l10nLabels.get(KEY).c_str())

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

- (instancetype)init:(BOOL)pinpad
{
    if (self = [super init]) {
        if (![NSBundle loadNibNamed:pinpad ? @"PINPadDialog" : @"PINDialog" owner:self]) {
            self = nil;
            return self;
        }
        if (pinpad) {
            [progressBar setDoubleValue:30];
            [progressBar startAnimation:self];
        }
        else {
            okButton.title = _L("sign");
            cancelButton.title = _L("cancel");
        }
        [pinFieldLabel setTitleWithMnemonic:_L(pinpad ? "enter PIN2 pinpad" : "enter PIN2")];
        pinPanel.title =_L("signing");
    }
    return self;
}

+ (NSDictionary *)show:(NSDictionary*)params
{
    if (!params[@"hash"] || !params[@"cert"] || [params[@"hash"] length] % 2 == 1) {
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

    PKCS11CardManager manager;
    time_t currentTime = DateUtils::now();
    std::unique_ptr<PKCS11CardManager> selected;
    for (auto &token : manager.getAvailableTokens()) {
        selected.reset(manager.getManagerForReader(token));
        if (BinaryUtils::hex2bin([params[@"cert"] UTF8String]) == selected->getSignCert() &&
            currentTime <= selected->getValidTo()) {
            break;
        }
        selected.reset();
    }

    if (!selected) {
        return @{@"result": @"invalid_argument"};
    }

    int retriesLeft = selected->getPIN2RetryCount();
    if (retriesLeft == 0) {
        return @{@"result": @"pin_blocked"};
    }

    PINPanel *dialog = [[PINPanel alloc] init:selected->isPinpad()];
    if (!dialog) {
        return @{@"result": @"technical_error"};
    }

    std::vector<unsigned char> signature;
    std::future<void> future;
    NSTimer *timer;
    if (selected->isPinpad()) {
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

    [dialog->nameLabel setTitleWithMnemonic:@((selected->getCardName() + ", " + selected->getPersonalCode()).c_str())];
    if (retriesLeft < 3) {
        [dialog->messageField setTitleWithMnemonic:[NSString stringWithFormat:@"%@ %u", _L("tries left"), retriesLeft]];
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
    okButton.enabled = pinField.stringValue.length >= 5;
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