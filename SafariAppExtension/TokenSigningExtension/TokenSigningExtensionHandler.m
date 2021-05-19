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

#import <SafariServices/SafariServices.h>
#import <AppKit/AppKit.h>

#import "TokenSigning.h"

static NSMutableDictionary<NSString *, SFSafariPage *> *pages;

@interface TokenSigningExtensionHandler : SFSafariExtensionHandler
@end

@implementation TokenSigningExtensionHandler

- (void)messageReceivedWithName:(NSString *)messageName fromPage:(SFSafariPage *)page userInfo:(NSDictionary *)userInfo {
    // This method will be called when a content script provided by your extension calls safari.extension.dispatchMessage("message").
    BOOL isRunning = [NSRunningApplication runningApplicationsWithBundleIdentifier:TokenSigningApp].count > 0;
    NSLog(@"TokenSigning isRunning: %d", isRunning);
    if (!isRunning) {
        NSBundle *bundle = [NSBundle bundleForClass:TokenSigningExtensionHandler.class];
        NSString *path = bundle.bundlePath.stringByDeletingLastPathComponent.stringByDeletingLastPathComponent.stringByDeletingLastPathComponent;
        NSLog(@"TokenSigning  path: %@", path);
        BOOL isLaunched = [NSWorkspace.sharedWorkspace launchApplication:path];
        NSLog(@"TokenSigning launchApplication: %d", isLaunched);
    }
    if (!pages) {
         pages = [[NSMutableDictionary<NSString *, SFSafariPage *> alloc] init];
    }
    pages[userInfo[@"nonce"]] = page;
    [page getPagePropertiesWithCompletionHandler:^(SFSafariPageProperties *properties) {
        NSLog(@"TokenSigning: The extension received a message (%@) from a script injected into (%@) with userInfo (%@)", messageName, properties.url, userInfo);
        if (![messageName isEqualToString:TokenSigningMessage]) {
            return;
        }
        NSUserDefaults *defaults = [[NSUserDefaults alloc] initWithSuiteName:TokenSigningShared];
        [defaults setObject:userInfo forKey:userInfo[@"nonce"]];
        [defaults synchronize];
        [NSDistributedNotificationCenter.defaultCenter postNotificationName:TokenSigning object:userInfo[@"nonce"] userInfo:nil deliverImmediately:YES];
    }];
}

- (void)messageReceivedFromContainingAppWithName:(NSString *)messageName userInfo:(NSDictionary<NSString *,id> *)userInfo {
    NSLog(@"TokenSigning: The extension received a message (%@) from a application with userInfo (%@)", messageName, userInfo);
    SFSafariPage *page = pages[userInfo[@"nonce"]];
    [pages removeObjectForKey:userInfo[@"nonce"]];
    [page dispatchMessageToScriptWithName:messageName userInfo:userInfo];
}

@end
