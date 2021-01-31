/*
 * Safari Token Signing
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
#import "PINDialog.h"
#import "Labels.h"
#import "../TokenSigningExtension/TokenSigning.h"

#import <SafariServices/SafariServices.h>

@interface AppDelegate : NSObject <NSApplicationDelegate>

@end

@implementation AppDelegate

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification {
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
    LSSharedFileListRef list = LSSharedFileListCreate(kCFAllocatorDefault, kLSSharedFileListSessionLoginItems, nil);
    if (list) {
        bool found = false;
        CFURLRef url = CFBundleCopyBundleURL(CFBundleGetMainBundle());
        UInt32 seedValue = 0;
        CFArrayRef loginItemsArray = LSSharedFileListCopySnapshot(list, &seedValue);
        for (id item in (__bridge NSArray *)loginItemsArray) {
            LSSharedFileListItemRef itemRef = (__bridge LSSharedFileListItemRef)item;
            if (CFStringRef tmp = LSSharedFileListItemCopyDisplayName(itemRef)) {
                if ([(__bridge NSString*)tmp containsString:@"TokenSigningApp"]) {
                    NSLog(@"Removing TokenSigningApp LoginItem");
                    LSSharedFileListItemRemove(list, itemRef);
                    CFRelease(tmp);
                    continue;
                }
                CFRelease(tmp);
            }
            if (CFURLRef tmp = LSSharedFileListItemCopyResolvedURL(itemRef, 0, nil)) {
                if ([[(__bridge NSURL*)tmp path] isEqualToString:[(__bridge NSURL*)url path]])
                    found = true;
                CFRelease(tmp);
            }
        }
        CFRelease(loginItemsArray);

        if (!found) {
            NSDictionary *props = @{(__bridge id)kLSSharedFileListLoginItemHidden: @(YES)};
            LSSharedFileListItemRef item = LSSharedFileListInsertItemURL(
                list, kLSSharedFileListItemLast, nil, nil, url, (__bridge CFDictionaryRef)props, nil);
            if (item) {
                CFRelease(item);
            }
        }
        CFRelease(url);
        CFRelease(list);
    }
#pragma clang diagnostic pop
    [self checkExtensionState];
    [NSDistributedNotificationCenter.defaultCenter addObserver:self selector:@selector(notificationEvent:) name:TokenSigning object:nil];
}

- (void)applicationDidBecomeActive:(NSNotification *)notification {
    [self checkExtensionState];
}

- (void)checkExtensionState {
    [SFSafariExtensionManager getStateOfSafariExtensionWithIdentifier:TokenSigningExtension completionHandler:^(SFSafariExtensionState *state, NSError *error) {
        NSLog(@"Extension state %@, error %@", @(state ? state.enabled : 0), error);
        if (!state.enabled) {
            [SFSafariApplication showPreferencesForExtensionWithIdentifier:TokenSigningExtension completionHandler:nil];
        }
    }];
}

- (void)notificationEvent:(NSNotification *)notification {
    NSUserDefaults *defaults = [[NSUserDefaults alloc] initWithSuiteName:TokenSigningShared];
    NSMutableDictionary *resp = [[defaults dictionaryForKey:notification.object] mutableCopy];
    [defaults removeObjectForKey:notification.object];
    [defaults synchronize];
    NSLog(@"request %@", resp);
    resp[@"src"] = @"background.js";
    resp[@"extension"] = [NSString stringWithFormat:@"%@.%@", NSBundle.mainBundle.infoDictionary[@"CFBundleShortVersionString"], NSBundle.mainBundle.infoDictionary[@"CFBundleVersion"]];

    if (resp[@"lang"]) {
        Labels::l10n.setLanguage([resp[@"lang"] UTF8String]);
    }

    if ([resp[@"type"] isEqualToString:@"VERSION"]) {
        resp[@"version"] = resp[@"extension"];
        resp[@"result"] = @"ok";
    }
    else if ([resp[@"origin"] compare:@"https" options:NSCaseInsensitiveSearch range:NSMakeRange(0, 5)]) {
        resp[@"result"] = @"not_allowed";
    }
    else if ([resp[@"type"] isEqualToString:@"CERT"]) {
        if ([@"AUTH" isEqualToString:resp[@"filter"]]) {
            resp[@"result"] = @"invalid_argument";
        } else {
            NSDictionary *cert = [CertificateSelection show];
            resp[@"cert"] = cert[@"cert"];
            resp[@"result"] = cert[@"result"];
        }
    }
    else if ([resp[@"type"] isEqualToString:@"SIGN"]) {
        NSDictionary *sign = [PINPanel show:resp cert:resp[@"cert"]];
        resp[@"signature"] = sign[@"signature"];
        resp[@"result"] = sign[@"result"];
    }
    else {
        resp[@"result"] = @"invalid_argument";
    }
    if (!resp[@"result"]) {
        resp[@"result"] = @"ok";
    }
    NSLog(@"response %@", resp);
    [SFSafariApplication dispatchMessageWithName:TokenSigningMessage toExtensionWithIdentifier:TokenSigningExtension userInfo:resp completionHandler:nil];
}

@end
