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
            if (data.length == 0) {
                exit(0);
                return;
            }
            [input waitForDataInBackgroundAndNotify];

            for (NSUInteger pos = 0; pos < data.length;) {
                uint32_t size = 0;
                [data getBytes:&size range:NSMakeRange(pos, sizeof(size))];
                if (size > 8*1024) {
                    exit(0);
                    return;
                }

                NSData *json = [data subdataWithRange:NSMakeRange(pos + 4, size)];
                pos += 4 + size;

                NSError *error;
                NSDictionary *dict = [NSJSONSerialization JSONObjectWithData:json options:0 error:&error];
                NSString *nonce = dict[@"nonce"];
                if (error) {
                    NSLog(@"Message (%u): %@", size, [[NSString alloc] initWithData:json encoding:NSUTF8StringEncoding]);
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
                json = [NSJSONSerialization dataWithJSONObject:resp options:0 error:&error];

                size = (uint32_t)json.length;
                NSData *sizeout = [NSData dataWithBytes:&size length:sizeof(size)];
                [[NSFileHandle fileHandleWithStandardOutput] writeData:sizeout];
                [[NSFileHandle fileHandleWithStandardOutput] writeData:json];
                
                if ([resp[@"result"] isEqualTo:@"invalid_argument"]) {
                    exit(1);
                }
            }
        }];
        [[NSRunLoop mainRunLoop] run];
    }
    return EXIT_SUCCESS;
}
