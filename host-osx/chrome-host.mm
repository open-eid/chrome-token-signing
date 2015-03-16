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

static void write(NSDictionary *data, NSString *nonce)
{
    NSMutableDictionary *resp = [NSMutableDictionary dictionaryWithDictionary:data];
    if (nonce) {
        resp[@"nonce"] = nonce;
    }
    if (!data[@"result"]) {
        resp[@"result"] = @"ok";
    }
    resp[@"ver"] = @(1);
    NSError *error;
    NSData *json = [NSJSONSerialization dataWithJSONObject:resp options:0 error:&error];

    uint32_t size = (uint32_t)json.length;
    NSData *sizeout = [NSData dataWithBytes:&size length:sizeof(size)];
    [NSFileHandle.fileHandleWithStandardOutput writeData:sizeout];
    [NSFileHandle.fileHandleWithStandardOutput writeData:json];
}

int main(int argc, const char * argv[]) {
    @autoreleasepool {
        /* -[NSFileManager waitForDataInBackgroundAndNotify] doc say I need
         an "active run loop".  I don't know what they mean by "active". */
        [NSRunLoop.mainRunLoop runUntilDate:[NSDate dateWithTimeIntervalSinceNow:1]];

        __block NSString *cert, *origin;
        NSFileHandle *input = NSFileHandle.fileHandleWithStandardInput;
        [input waitForDataInBackgroundAndNotify];
        NSNotificationCenter *dc = NSNotificationCenter.defaultCenter;
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
                    write(@{@"result": @"invalid_argument"}, nil);
                    exit(1);
                    return;
                }

                NSData *json = [data subdataWithRange:NSMakeRange(pos + 4, size)];
                pos += 4 + size;

                NSError *error;
                NSDictionary *dict = [NSJSONSerialization JSONObjectWithData:json options:0 error:&error];
                NSDictionary *result;
                if (error) {
                    NSLog(@"Message (%u): %@", size, [[NSString alloc] initWithData:json encoding:NSUTF8StringEncoding]);
                    write(@{@"result": @"invalid_argument"}, nil);
                    exit(1);
                    return;
                }
                else if(!dict[@"nonce"] || !dict[@"type"] || !dict[@"origin"]) {
                    write(@{@"result": @"invalid_argument"}, dict[@"nonce"]);
                    exit(1);
                    return;
                }
                else {
                    if (!origin) {
                        origin = dict[@"origin"];
                    } else if (![origin isEqualToString:dict[@"origin"]]) {
                        write(@{@"result": @"invalid_argument"}, nil);
                        exit(1);
                        return;
                    }
                    if (dict[@"lang"]) {
                        Labels::l10n.setLanguage([dict[@"lang"] UTF8String]);
                    }
                    if ([dict[@"type"] isEqualToString:@"VERSION"]) {
                        result = @{@"version": [NSBundle mainBundle].infoDictionary[@"CFBundleShortVersionString"]};
                    }
                    else if ([dict[@"origin"] compare:@"https" options:NSCaseInsensitiveSearch range:NSMakeRange(0, 5)]) {
                        result = @{@"result": @"not_allowed"};
                    }
                    else if([dict[@"type"] isEqualToString:@"CERT"]) {
                        result = [CertificateSelection show];
                        cert = (NSString*)result[@"cert"];
                    }
                    else if ([dict[@"type"] isEqualToString:@"SIGN"]) {
                        result = [PINPanel show:dict cert:cert];
                    }
                    else {
                        result = @{@"result": @"invalid_argument"};
                    }
                }
                write(result, dict[@"nonce"]);
            }
        }];
        [NSRunLoop.mainRunLoop run];
    }
    return EXIT_SUCCESS;
}
