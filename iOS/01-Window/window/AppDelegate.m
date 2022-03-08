//
//  AppDelegate.m
//  window
//
//  Created by Akshay Patel on 9/24/21.
//

#import "AppDelegate.h"
#import "ViewController.h"
#import "MyView.h"

@implementation AppDelegate
{
    @private
        UIWindow *window;
        ViewController *view_controller;
        MyView *view;
}

- (BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions
{
    // Override point for customization after application launch.
    CGRect win_rect = [[UIScreen mainScreen] bounds];
    window = [[UIWindow alloc] initWithFrame:win_rect];
    
    view_controller = [[ViewController alloc] init];
    [window setRootViewController:view_controller];
    
    view = [[MyView alloc] initWithFrame:win_rect];
    [view_controller setView:view];
    
    [window makeKeyAndVisible];
    [view release];
    
    return YES;
}

- (void)applicationWillResignActive:(UIApplication *)application
{
    //code
}

- (void)applicationDidEnterBackground:(UIApplication *)application
{
    //code
}

- (void)applicationWillEnterForeground:(UIApplication *)application
{
    //code
}

- (void)applicationDidBecomeActive:(UIApplication *)application
{
    //code
}

- (void)applicationWillTerminate:(UIApplication *)application
{
    //code
}

- (void)dealloc
{
    //code
    [super dealloc];
    [view release];
    [view_controller release];
    [window release];
}

@end
