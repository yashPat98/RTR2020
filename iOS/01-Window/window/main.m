//
//  main.m
//  window
//
//  Created by Akshay Patel on 9/24/21.
//

#import <UIKit/UIKit.h>
#import "AppDelegate.h"

int main(int argc, char * argv[])
{
    //variable declarations
    int ret;
    
    //code
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
    
    NSString* appDelegateClassName = NSStringFromClass([AppDelegate class]);
    ret = UIApplicationMain(argc, argv, nil, appDelegateClassName);
    
    [pool release];
    
    return (ret);
}
