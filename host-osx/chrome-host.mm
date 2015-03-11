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
#import "PINDialog.h"
#import "Labels.h"

int main(int argc, const char * argv[]) {
    @autoreleasepool {
        /* -[NSFileManager waitForDataInBackgroundAndNotify] doc say I need
         an "active run loop".  I don't know what they mean by "active". */
        [[NSRunLoop mainRunLoop] runUntilDate:[NSDate dateWithTimeIntervalSinceNow:1]];

        NSFileHandle* input = [NSFileHandle fileHandleWithStandardInput];
        [input waitForDataInBackgroundAndNotify];
        NSNotificationCenter *dc = [NSNotificationCenter defaultCenter];
        [dc addObserverForName:NSFileHandleDataAvailableNotification object:input queue:nil usingBlock:^(NSNotification *note) {
            NSData *data = [input availableData];
            [input waitForDataInBackgroundAndNotify];
            if (data.length == 0) {
                return;
            }

            uint32_t size = 0;
            [data getBytes:&size length:sizeof(size)];
            if (size > 8*1024) {
                exit(0);
                return;
            }

            data = [data subdataWithRange:NSMakeRange(4, data.length - 4)];
            NSError *error;
            NSDictionary *dict = [NSJSONSerialization JSONObjectWithData:data options:0 error:&error];
            NSString *nonce = dict[@"nonce"];
            if (error) {
                dict = @{@"result": @"invalid_argument", @"message": error.localizedDescription};
            }
            else if(!dict[@"nonce"] || !dict[@"type"] || !dict[@"origin"]) {
                dict = @{@"result": @"invalid_argument"};
            }
            else {
                if (dict[@"lang"]) {
                    l10nLabels.setLanguage([dict[@"lang"] UTF8String]);
                }
                if ([dict[@"type"] isEqualToString:@"VERSION"]) {
                    dict = @{@"version": [NSBundle mainBundle].infoDictionary[@"CFBundleShortVersionString"]};
                }
                else if ([dict[@"origin"] compare:@"https" options:NSCaseInsensitiveSearch range:NSMakeRange(0, 5)]) {
                    dict = @{@"result": @"not_allowed"};
                }
                else if([dict[@"type"] isEqualToString:@"CERT"]) {
                    dict = [CertificateSelection show];
                }
                else if ([dict[@"type"] isEqualToString:@"SIGN"]) {
                    dict = [PINPanel show:dict];
                }
                else {
                    dict = @{@"result": @"invalid_argument"};
                }
            }

            NSMutableDictionary *resp = [NSMutableDictionary dictionaryWithDictionary:dict];
            if (nonce) {
                resp[@"nonce"] = nonce;
            }
            if (!dict[@"result"]) {
                resp[@"result"] = @"ok";
            }
            resp[@"ver"] = @(1);
            data = [NSJSONSerialization dataWithJSONObject:resp options:0 error:&error];

            size = (uint32_t)data.length;
            NSData *sizeout = [NSData dataWithBytes:&size length:sizeof(size)];
            [[NSFileHandle fileHandleWithStandardOutput] writeData:sizeout];
            [[NSFileHandle fileHandleWithStandardOutput] writeData:data];
        }];
        [[NSRunLoop mainRunLoop] run];
    }
    return EXIT_SUCCESS;
}
