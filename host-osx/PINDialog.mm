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

static NSTouchBarItemIdentifier touchBarItemGroupId = @"ee.ria.chrome-token-signing.touchbar.group";
static NSTouchBarItemIdentifier touchBarItemOkId = @"ee.ria.chrome-token-signing.touchbar.ok";
static NSTouchBarItemIdentifier touchBarItemCancelId = @"ee.ria.chrome-token-signing.touchbar.cancel";

@interface PINPanel () <NSTouchBarDelegate>
@end

@implementation PINPanel {
    IBOutlet NSPanel *window;
    IBOutlet NSButton *okButton;
    IBOutlet NSButton *cancelButton;
    IBOutlet NSTextField *nameLabel;
    IBOutlet NSTextField *messageField;
    IBOutlet NSSecureTextField *pinField;
    IBOutlet NSTextField *pinFieldLabel;
    IBOutlet NSProgressIndicator *progressBar;
    unsigned long minPinLen;
}

- (instancetype)init:(NSString*)label pinpad:(BOOL)pinpad
{
    if (self = [super init]) {
        if (![[NSBundle bundleForClass:PINPanel.class] loadNibNamed:pinpad ? @"PINPadDialog" : @"PINDialog" owner:self topLevelObjects:nil]) {
            self = nil;
            return self;
        }
        if (pinpad) {
            progressBar.doubleValue = 30;
            [progressBar startAnimation:self];
        }
        else {
            cancelButton.title = _L("cancel");
        }
        window.touchBar = [self makeTouchBar];
        pinFieldLabel.stringValue = label;
    }
    return self;
}

+ (NSDictionary *)show:(NSDictionary*)params cert:(NSString *)cert
{
    if (!params[@"hash"] || !params[@"cert"] || [params[@"hash"] length] % 2 == 1) {
        return @{@"result": @"invalid_argument"};
    }

    if (params[@"info"] && [params[@"info"] length] > 0) {
        if ([params[@"info"] length] > 500) {
            return @{@"result": @"technical_error"};
        }
        NSAlert *alert = [[NSAlert alloc] init];
        [alert addButtonWithTitle:@"OK"];
        [alert addButtonWithTitle:@"Cancel"];
        alert.messageText = params[@"info"];
        alert.alertStyle = NSAlertStyleInformational;
        alert.icon = [[NSImage alloc] initByReferencingFile:@"/System/Library/CoreServices/CoreTypes.bundle/Contents/Resources/AlertNoteIcon.icns"];
        if ([alert runModal] != NSAlertFirstButtonReturn) {
            return @{@"result": @"user_cancel"};
        }
    }

    PKCS11Path::Params p11 = PKCS11Path::getPkcs11ModulePath();
    std::unique_ptr<PKCS11CardManager> pkcs11;
    PKCS11CardManager::Token selected;
    std::vector<unsigned char> hash;
    try {
        hash = BinaryUtils::hex2bin([params[@"hash"] UTF8String]);
        pkcs11.reset(new PKCS11CardManager(p11.path));
        for (const PKCS11CardManager::Token &token : pkcs11->tokens()) {
            if (BinaryUtils::hex2bin(cert.UTF8String) == token.cert) {
                selected = token;
                break;
            }
        }
    }
    catch(const BaseException &e) {
        _log("Exception: %s", e.what());
        return @{@"result": @(e.getErrorCode().c_str())};
    }

    if (selected.cert.empty()) {
        return @{@"result": @"invalid_argument"};
    }

    CFDataRef data = CFDataCreateWithBytesNoCopy(nil, selected.cert.data(), selected.cert.size(), kCFAllocatorNull);
    SecCertificateRef x509 = SecCertificateCreateWithData(nil, data);
    CFRelease(data);
    NSDictionary *dict = CFBridgingRelease(SecCertificateCopyValues(x509, nil, nil));
    CFRelease(x509);

    bool isInitialCheck = true;
    for (int retriesLeft = selected.retry; retriesLeft > 0; )
    {
        NSString *label;
        NSNumber *ku = dict[(__bridge id)kSecOIDKeyUsage][(__bridge id)kSecPropertyKeyValue];
        if (ku.unsignedIntValue & kSecKeyUsageNonRepudiation) {
            label = [_L(selected.pinpad ? "sign PIN pinpad" : "sign PIN") stringByReplacingOccurrencesOfString:@"@PIN@" withString:@(p11.signPINLabel.c_str())];
        }
        else {
            label = [_L(selected.pinpad ? "auth PIN pinpad" : "auth PIN") stringByReplacingOccurrencesOfString:@"@PIN@" withString:@(p11.authPINLabel.c_str())];
        }
        PINPanel *dialog = [[PINPanel alloc] init:label pinpad:selected.pinpad];
        if (!dialog) {
            return @{@"result": @"technical_error"};
        }
        dialog->minPinLen = selected.minPinLen;
        for (NSDictionary *item in dict[(__bridge id)kSecOIDX509V1SubjectName][(__bridge id)kSecPropertyKeyValue]) {
            if ([item[(__bridge id)kSecPropertyKeyLabel] isEqualToString:(__bridge id)kSecOIDCommonName]) {
                dialog->nameLabel.stringValue = item[(__bridge id)kSecPropertyKeyValue];
            }
        }

        NSDictionary *pinpadresult;
        std::future<void> future;
        NSTimer *timer;
        if (selected.pinpad) {
            timer = [NSTimer scheduledTimerWithTimeInterval:1.0 target:dialog selector:@selector(handleTimerTick:) userInfo:nil repeats:YES];
            [NSRunLoop.currentRunLoop addTimer:timer forMode:NSModalPanelRunLoopMode];
            future = std::async(std::launch::async, [&] {
                try {
                    pinpadresult = @{@"signature": @(BinaryUtils::bin2hex(pkcs11->sign(selected, hash, nullptr)).c_str()), @"result": @"ok"};
                    [NSApp stopModal];
                }
                catch(const UserCancelledException &) {
                    [NSApp abortModal];
                }
                catch(const AuthenticationError &) {
                    --retriesLeft;
                    [NSApp stopModal];
                }
                catch(const PinBlockedException &) {
                    retriesLeft = 0;
                    [NSApp stopModal];
                }
                catch(const AuthenticationBadInput &) {
                    [NSApp stopModal];
                }
                catch(const BaseException &e) {
                    _log("Exception: %s", e.what());
                    pinpadresult = @{@"result": @(e.getErrorCode().c_str())};
                    [NSApp stopModal];
                }
            });
        }

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

        if (selected.pinpad) {
            future.wait();
            if (pinpadresult) {
                return pinpadresult;
            }
        }
        else {
            try {
                std::vector<unsigned char> signature = pkcs11->sign(selected, hash, dialog->pinField.stringValue.UTF8String);
                return @{@"signature": @(BinaryUtils::bin2hex(signature).c_str()), @"result": @"ok"};
            }
            catch(const AuthenticationBadInput &) {
            }
            catch(const AuthenticationError &) {
                --retriesLeft;
            }
            catch(const PinBlockedException &) {
                retriesLeft = 0;
            }
            catch(const BaseException &e) {
                _log("Exception: %s", e.what());
                return @{@"result": @(e.getErrorCode().c_str())};
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
    if (okButton.enabled != pinField.stringValue.length >= minPinLen)
    {
        okButton.enabled = pinField.stringValue.length >= minPinLen;
        window.touchBar = [self makeTouchBar];
    }
}

- (void)handleTimerTick:(NSTimer*)timer
{
    [progressBar incrementBy:-1];
    if (progressBar.doubleValue <= 0) {
        [progressBar stopAnimation:self];
        [NSApp abortModal];
    }
}

#pragma mark - NSTouchBarProvider

- (NSTouchBar *)makeTouchBar
{
    NSTouchBar *touchBar = [[NSTouchBar alloc] init];
    touchBar.delegate = self;
    touchBar.defaultItemIdentifiers = @[touchBarItemGroupId];
    touchBar.principalItemIdentifier = touchBarItemGroupId;
    return touchBar;
}

#pragma mark - NSTouchBarDelegate

- (NSTouchBarItem *)touchBar:(NSTouchBar *)touchBar makeItemForIdentifier:(NSTouchBarItemIdentifier)identifier
{
    if ([identifier isEqualToString:touchBarItemGroupId])
    {
        NSArray *items = @[[self touchBar:touchBar makeItemForIdentifier:touchBarItemCancelId],
                           [self touchBar:touchBar makeItemForIdentifier:touchBarItemOkId]];
        return [NSGroupTouchBarItem groupItemWithIdentifier:identifier items:items];
    }
    if ([identifier isEqualToString:touchBarItemOkId])
    {
        NSButton *button = [NSButton buttonWithTitle:@"OK" target:self action:@selector(okClicked:)];
        button.enabled = okButton.enabled;
        if (okButton.enabled)
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

@end
