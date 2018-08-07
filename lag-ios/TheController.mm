//
//  TheController.m
//  gallery-ios
//
//  Created by Guillem on 18/04/2018.
//  Copyright © 2018 Nifty Ninja. All rights reserved.
//

#import "TheController.h"
#include "../code/Engine/engine.hpp"
#include "../code/Engine/window.hpp"
#include "../code/Engine/opengl.hpp"
#include "../code/main.hpp"

extern Vec2i windowSize;
extern f32 TIME_STEP;
Angle screen_tilt_angle;

@interface TheController ()

@property (strong, nonatomic) EAGLContext *context;

@end

GLint defaultFBO;

bool keys[KEYS_NUM];
StaticArray<Event, MAX_EVENTS_PER_LOOP> events(0);


@implementation TheController

-(void) observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary *)change context:(void *)context {
    static float last_volume = 0.f;
    if ([keyPath isEqual:@"outputVolume"]) {
        float volume = [[MPMusicPlayerController applicationMusicPlayer] volume];
        if(volume > last_volume){
            keys[KEYS_UP] = true;
        }else if(volume < last_volume){
            keys[KEYS_UP] = false;
        }
        last_volume = volume;
    }
}

- (void)viewDidLoad {
    [super viewDidLoad];
    
    self.context = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES3];
    
    if (!self.context) {
        NSLog(@"Failed to create ES context");
    }
    
    GLKView *view = (GLKView *)self.view;
    view.context = self.context;
    [EAGLContext setCurrentContext:self.context];
    
    int height = [[UIScreen mainScreen] bounds].size.height;
    int width  = [[UIScreen mainScreen] bounds].size.width;
    
    [view bindDrawable];
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &defaultFBO);
    
    windowSize.x = 2*width;
    windowSize.y = 2*height;
    TIME_STEP = 1.f/30.f;
    
    initEngine();
    initGame();
    
    events.size = 0;
}

/*- (void)centralManagerDidUpdateState:(CBCentralManager *)central {
    NSLog(@"didUpdateState");
    switch (central.state) {
        case CBManagerStatePoweredOn:
            // Scans for any peripheral
            [self.manager scanForPeripheralsWithServices:nil options:nil];
            break;
        default:
            NSLog(@"Central Manager did change state");
            break;
    }
}
- (void)centralManager:(CBCentralManager *)central didDiscoverPeripheral:(CBPeripheral *)peripheral advertisementData:(NSDictionary *)advertisementData RSSI:(NSNumber *)RSSI {
    // Stops scanning for peripheral
    [self.manager stopScan];
    NSLog(@"Found peripheral %@", peripheral);
    
    if (self.peripheral != peripheral) {
        self.peripheral = peripheral;
        NSLog(@"Connecting to peripheral %@", peripheral);
        // Connects to the discovered peripheral
        [self.manager connectPeripheral:peripheral options:nil];
    }
}
- (void)centralManager:(CBCentralManager *)central didConnectPeripheral:(CBPeripheral *)peripheral {
    // Clears the data that we may already have
    [self.data setLength:0];
    // Sets the peripheral delegate
    [self.peripheral setDelegate:self];
    // Asks the peripheral to discover the service
    //[self.peripheral discoverServices:@[ [CBUUID UUIDWithString:kServiceUUID] ]];
}*/

- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation
{
    return UIInterfaceOrientationIsLandscape(interfaceOrientation);
}


#pragma mark - GLKViewDelegate

- (void)glkView:(GLKView *)view drawInRect:(CGRect)rect {
    //[self centralManagerDidUpdateState:self.manager];
    gameLogic(keys, events);
    gameDraw();
    //glClearColor(0, 1, 0, 1);
    //glClear(GL_COLOR_BUFFER_BIT);
}

- (void)update {
    
}

- (UIRectEdge)preferredScreenEdgesDeferringSystemGestures{
    return UIRectEdgeAll;
}

- (void)touchesBegan:(NSSet *)touches withEvent:(UIEvent *)event {
    float height = [[UIScreen mainScreen] bounds].size.height;
    for(UITouch *touch in touches){
        CGPoint point = [touch locationInView:[self view]];
        point.x /= height;
        point.y /= height;
        if(point.y >= 0.78){
            if(point.x < 0.23){
                keys[KEYS_LEFT] = true;
            }else if(point.x < 0.44){
                keys[KEYS_RIGHT] = true;
            }else if(point.x > 0.78){
                keys[KEYS_SPACE] = true;
            }
        }
    }
}

- (void)touchesMoved:(NSSet *)touches withEvent:(UIEvent *)event {
    float height = [[UIScreen mainScreen] bounds].size.height;
    for(UITouch *touch in touches){
        CGPoint point = [touch locationInView:[self view]];
        point.x /= height;
        point.y /= height;
        CGPoint previous_point = [touch previousLocationInView:[self view]];
        previous_point.x /= height;
        previous_point.y /= height;
        if(previous_point.y >= 0.78){
            if(previous_point.x < 0.23){
                keys[KEYS_LEFT] = false;
            }else if(previous_point.x < 0.44){
                keys[KEYS_RIGHT] = false;
            }else if(previous_point.x > 0.78){
                keys[KEYS_SPACE] = false;
            }
        }
        if(point.y >= 0.78){
            if(point.x < 0.23){
                keys[KEYS_LEFT] = true;
            }else if(point.x < 0.44){
                keys[KEYS_RIGHT] = true;
            }else if(point.x > 0.78){
                keys[KEYS_SPACE] = true;
            }
        }
    }
}

- (void)touchesEnded:(NSSet *)touches withEvent:(UIEvent *)event {
    float height = [[UIScreen mainScreen] bounds].size.height;
    for(UITouch *touch in touches){
        CGPoint previous_point = [touch previousLocationInView:[self view]];
        previous_point.x /= height;
        previous_point.y /= height;
        if(previous_point.y >= 0.78){
            if(previous_point.x < 0.23){
                keys[KEYS_LEFT] = false;
            }else if(previous_point.x < 0.44){
                keys[KEYS_RIGHT] = false;
            }else if(previous_point.x > 0.78){
                keys[KEYS_SPACE] = false;
            }
        }
    }
}

- (void)didReceiveMemoryWarning {
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}

/*
 #pragma mark - Navigation
 
 // In a storyboard-based application, you will often want to do a little preparation before navigation
 - (void)prepareForSegue:(UIStoryboardSegue *)segue sender:(id)sender {
 // Get the new view controller using [segue destinationViewController].
 // Pass the selected object to the new view controller.
 }
 */

@end
