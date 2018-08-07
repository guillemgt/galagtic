//
//  TheController.h
//  gallery-ios
//
//  Created by Guillem on 18/04/2018.
//  Copyright © 2018 Nifty Ninja. All rights reserved.
//

#import <GLKit/GLKit.h>
#import <CoreMotion/CoreMotion.h>
#import <CoreLocation/CoreLocation.h>
#import <AVFoundation/AVFoundation.h>
#import <MediaPlayer/MediaPlayer.h>

@interface TheController : GLKViewController// <CBCentralManagerDelegate, CBPeripheralDelegate>

//@property (nonatomic, strong) CBCentralManager *manager;
//@property (nonatomic, strong) NSMutableData *data;
//@property (nonatomic, strong) CBPeripheral *peripheral;

@property (strong, nonatomic) CMMotionManager *motionManager;
@property (strong, nonatomic) NSOperationQueue *deviceQueue;
@property (strong, nonatomic) CLLocationManager *locationManager;

@end
