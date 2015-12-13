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

#import "PINDialog.h"

#import "BinaryUtils.h"
#import "PKCS11CardManager.h"
#import "Labels.h"
#import "PKCS11Path.h"

#include <future>

#define _L(KEY) @(Labels::l10n.get(KEY).c_str())

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
            progressBar.doubleValue = 30;
            [progressBar startAnimation:self];
        }
        else {
            okButton.title = _L("sign");
            cancelButton.title = _L("cancel");
        }
        pinFieldLabel.stringValue = _L(pinpad ? "enter PIN2 pinpad" : "enter PIN2");
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
        case BINARY_SHA1_LENGTH:
        case BINARY_SHA224_LENGTH:
        case BINARY_SHA256_LENGTH:
        case BINARY_SHA384_LENGTH:
        case BINARY_SHA512_LENGTH: break;
        default: return @{@"result": @"invalid_argument"};
    }
    std::string pkcs11ModulePath(PKCS11Path::getPkcs11ModulePath());
    std::unique_ptr<PKCS11CardManager> selected;
    try {
        for (auto &token : PKCS11CardManager::instance(pkcs11ModulePath)->getAvailableTokens()) {
            selected.reset(PKCS11CardManager::instance(pkcs11ModulePath)->getManagerForReader(token));
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

    bool isInitialCheck = true;
    for (int retriesLeft = selected->getPIN2RetryCount(); retriesLeft > 0; ) {
        PINPanel *dialog = [[PINPanel alloc] init:selected->isPinpad()];
        if (!dialog) {
            return @{@"result": @"technical_error"};
        }

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

        dialog->nameLabel.stringValue = @((selected->getCardName() + ", " + selected->getPersonalCode()).c_str());
        if (retriesLeft < 3) {
            dialog->messageField.stringValue = [NSString stringWithFormat:@"%@%@ %u",
                                                (isInitialCheck ? @"" : _L("incorrect PIN2")),
                                                _L("tries left"),
                                                retriesLeft];
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
    // replace content with its intValue
    pinField.stringValue = [[pinField.stringValue componentsSeparatedByCharactersInSet:
                             NSCharacterSet.decimalDigitCharacterSet.invertedSet]
                            componentsJoinedByString:@""];
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
