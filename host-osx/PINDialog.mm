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

#import "BinaryUtils.h"
#import "PKCS11CardManager.h"
#import "Labels.h"

#include <future>

#define _L(KEY) @(Labels::l10n.get(KEY).c_str())

@interface OnlyIntegerValueFormatter : NSNumberFormatter
@end

@implementation OnlyIntegerValueFormatter

- (BOOL)isPartialStringValid:(NSString*)partialString newEditingString:(NSString**)newString errorDescription:(NSString**)error
{
    if(partialString.length == 0) {
        return YES;
    }
    NSScanner *scanner = [NSScanner scannerWithString:partialString];
    return [scanner scanInt:0] && scanner.isAtEnd;
}

@end

@interface PINPanel () {
    IBOutlet NSPanel *window;
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
        window.title =_L("signing");
    }
    return self;
}

+ (NSDictionary *)show:(NSDictionary*)params cert:(NSString *)cert
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

    std::unique_ptr<PKCS11CardManager> selected;
    try {
        for (auto &token : PKCS11CardManager::instance()->getAvailableTokens()) {
            selected.reset(PKCS11CardManager::instance()->getManagerForReader(token));
            if (BinaryUtils::hex2bin(cert.UTF8String) == selected->getSignCert()) {
                break;
            }
            selected.reset();
        }
    }
    catch(const std::runtime_error &) {
        return @{@"result": @"technical_error"};
    }

    if (!selected) {
        return @{@"result": @"invalid_argument"};
    }

    for (int retriesLeft = selected->getPIN2RetryCount(); retriesLeft > 0; ) {
        PINPanel *dialog = [[PINPanel alloc] init:selected->isPinpad()];
        if (!dialog) {
            return @{@"result": @"technical_error"};
        }

        bool isInitialCheck = true;
        NSDictionary *pinpadresult;
        std::future<void> future;
        NSTimer *timer;
        if (selected->isPinpad()) {
            timer = [NSTimer scheduledTimerWithTimeInterval:1.0 target:dialog selector:@selector(handleTimerTick:) userInfo:nil repeats:YES];
            [[NSRunLoop currentRunLoop] addTimer:timer forMode:NSModalPanelRunLoopMode];
            future = std::async(std::launch::async, [&]() {
                try {
                    std::vector<unsigned char> signature = selected->sign(hash, nullptr);
                    pinpadresult = @{@"signature":@(BinaryUtils::bin2hex(signature).c_str())};
                    [NSApp stopModal];
                }
                catch(const UserCanceledError &) {
                    [NSApp abortModal];
                }
                catch(const AuthenticationError &) {
                    --retriesLeft;
                    [NSApp stopModal];
                }
                catch(const AuthenticationBadInput &) {
                    [NSApp stopModal];
                }
                catch(const std::runtime_error &) {
                    pinpadresult = @{@"result": @"technical_error"};
                    [NSApp stopModal];
                }
            });
        }

        [dialog->nameLabel setTitleWithMnemonic:@((selected->getCardName() + ", " + selected->getPersonalCode()).c_str())];
        if (retriesLeft < 3) {
            [dialog->messageField setTitleWithMnemonic:[NSString stringWithFormat:@"%@%@ %u",
                                                        (!isInitialCheck ? _L("incorrect PIN2") : @""),
                                                        _L("tries left"),
                                                        retriesLeft]];
        }
        isInitialCheck = false;

        [NSApp activateIgnoringOtherApps:YES];
        NSModalResponse result = [NSApp runModalForWindow:dialog->window];
        [dialog->window close];

        if (timer) {
            [timer invalidate];
            timer = nil;
        }

        if (result == NSModalResponseAbort) {
            return @{@"result": @"user_cancel"};
        }

        if (selected->isPinpad()) {
            future.wait();
            if (pinpadresult) {
                return pinpadresult;
            }
        }
        else {
            try {
                std::vector<unsigned char> signature = selected->sign(hash, dialog->pinField.stringValue.UTF8String);
                return @{@"signature":@(BinaryUtils::bin2hex(signature).c_str())};
            }
            catch(const AuthenticationBadInput &) {
            }
            catch(const AuthenticationError &) {
                --retriesLeft;
            }
            catch(const std::runtime_error &) {
                return @{@"result": @"technical_error"};
            }
        }
    }
    return @{@"result": @"pin_blocked"};
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