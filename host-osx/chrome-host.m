/* Chrome Linux plugin
 *
 * This software is released under either the GNU Library General Public
 * License (see LICENSE.LGPL).
 *
 * Note that the only valid version of the LGPL license as far as this
 * project is concerned is the original GNU Library General Public License
 * Version 2.1, February 1999
 */

#import <Cocoa/Cocoa.h>

#import "CertificateSelection.h"
#import "PINDialog.h"

int main(int argc, const char * argv[]) {
    @autoreleasepool {
        /* -[NSFileManager waitForDataInBackgroundAndNotify] doc say I need
         an "active run loop".  I don't know what they mean by "active". */
        [[NSRunLoop mainRunLoop] runUntilDate:[NSDate dateWithTimeIntervalSinceNow:1]];

        NSFileHandle* input = [NSFileHandle fileHandleWithStandardInput];
        [input waitForDataInBackgroundAndNotify];
        NSNotificationCenter *dc = [NSNotificationCenter defaultCenter];
        [dc addObserverForName:NSFileHandleDataAvailableNotification object:input queue:nil usingBlock:^(NSNotification* note) {
            NSData *data = [input availableData];
            if ([data length] > 0) {
                uint32_t size = 0;
                [data getBytes:&size length:sizeof(size)];
                size = CFSwapInt32LittleToHost(size);
                if (size > 8*1024) {
                    exit(0);
                    return;
                }

                data = [data subdataWithRange:NSMakeRange(4, data.length - 4)];
                NSError *error;
                NSDictionary *dict = [NSJSONSerialization JSONObjectWithData:data options:0 error:&error];
                NSString *nonce = dict[@"nonce"];
                if (error) {
                    dict = @{@"returnCode": @5, @"message": error.localizedDescription};
                }
                else if ([@"VERSION" isEqualToString:dict[@"type"]]) {
                    dict = @{@"version": [NSBundle mainBundle].infoDictionary[@"CFBundleShortVersionString"]};
                }
                else if([@"CERT" isEqualToString:dict[@"type"]]) {
                    dict = [CertificateSelection show];
                }
                else if ([@"SIGN" isEqualToString:dict[@"type"]]) {
                    dict = [PINPanel show:dict];
                }
                NSMutableDictionary *resp = [NSMutableDictionary dictionaryWithDictionary:dict];
                if (nonce) {
                    resp[@"nonce"] = nonce;
                }
                data = [NSJSONSerialization dataWithJSONObject:resp options:0 error:&error];

                size = CFSwapInt32HostToLittle((uint32_t)data.length);
                NSData *sizeout = [NSData dataWithBytes:&size length:sizeof(size)];
                [[NSFileHandle fileHandleWithStandardOutput] writeData:sizeout];
                [[NSFileHandle fileHandleWithStandardOutput] writeData:data];
            }
            exit(0);
        }];
        [[NSRunLoop mainRunLoop] run];
    }
    return EXIT_SUCCESS;
}
