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

#import "CertificateSelection.h"
#import "PINDialog.h"
#import "Labels.h"
#import "Logger.h"

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
    _log("Response(%u) %s", size, (const char*)json.bytes);

    NSData *sizeout = [NSData dataWithBytes:&size length:sizeof(size)];
    [NSFileHandle.fileHandleWithStandardOutput writeData:sizeout];
    [NSFileHandle.fileHandleWithStandardOutput writeData:json];
}

int main(int argc, const char * argv[]) {
    @autoreleasepool {
        /* -[NSFileManager waitForDataInBackgroundAndNotify] doc say I need
         an "active run loop".  I don't know what they mean by "active". */
        [NSRunLoop.mainRunLoop runUntilDate:[NSDate dateWithTimeIntervalSinceNow:1]];

        __block NSString *cert, *origin, *version = [NSString stringWithFormat:@"%@.%@",
                                                     [NSBundle mainBundle].infoDictionary[@"CFBundleShortVersionString"],
                                                     [NSBundle mainBundle].infoDictionary[@"CFBundleVersion"]];
        _log("Starting native host %s", version.UTF8String);
        NSFileHandle *input = NSFileHandle.fileHandleWithStandardInput;
        [input waitForDataInBackgroundAndNotify];
        NSNotificationCenter *dc = NSNotificationCenter.defaultCenter;
        [dc addObserverForName:NSFileHandleDataAvailableNotification object:input queue:nil usingBlock:^(NSNotification *note) {
            NSData *data = [input availableData];
            if (data.length == 0) {
                return exit(0);
            }
            [input waitForDataInBackgroundAndNotify];

            for (NSUInteger pos = 0; pos < data.length;) {
                uint32_t size = 0;
                [data getBytes:&size range:NSMakeRange(pos, sizeof(size))];
                pos += sizeof(size);
                _log("Message size: %u", size);
                if (size > 8*1024) {
                    write(@{@"result": @"invalid_argument"}, nil);
                    return exit(1);
                }
                else if (data.length < pos + size) {
                    _log("Size (%u) exceeds available data (%u)", size, data.length - pos);
                    write(@{@"result": @"invalid_argument"}, nil);
                    return exit(1);
                }

                NSData *json = [data subdataWithRange:NSMakeRange(pos, size)];
                pos += size;
                _log("Message (%u): %s", size, (const char*)json.bytes);

                NSError *error;
                NSDictionary *dict = [NSJSONSerialization JSONObjectWithData:json options:0 error:&error];
                NSDictionary *result;
                if (error) {
                    write(@{@"result": @"invalid_argument"}, nil);
                    return exit(1);
                }

                if(!dict[@"nonce"] || !dict[@"type"] || !dict[@"origin"]) {
                    write(@{@"result": @"invalid_argument"}, dict[@"nonce"]);
                    return exit(1);
                }

                if (!origin) {
                    origin = dict[@"origin"];
                } else if (![origin isEqualToString:dict[@"origin"]]) {
                    write(@{@"result": @"invalid_argument"}, nil);
                    return exit(1);
                }

                if (dict[@"lang"]) {
                    Labels::l10n.setLanguage([dict[@"lang"] UTF8String]);
                }

                if ([dict[@"type"] isEqualToString:@"VERSION"]) {
                    result = @{@"version": version};
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

                write(result, dict[@"nonce"]);
            }
        }];
        [NSRunLoop.mainRunLoop run];
    }
    return EXIT_SUCCESS;
}
