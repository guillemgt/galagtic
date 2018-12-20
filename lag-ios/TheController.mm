//
//  TheController.m
//  gallery-ios
//
//  Created by Guillem on 18/04/2018.
//  Copyright © 2018 Nifty Ninja. All rights reserved.
//

#import "TheController.h"
#include "../code/basecode/basecode.hpp"
#include "../code/basecode/window.hpp"
#include "../code/basecode/opengl.hpp"
#include "../code/main.hpp"
#include "../code/menu.hpp"

extern Vec2i window_size;
extern f32 TIME_STEP;
Angle screen_tilt_angle;
Vec2 touch_point;

void ios_do_small_haptic_feedback(){
    UISelectionFeedbackGenerator *generator = [[UISelectionFeedbackGenerator alloc] init];
    [generator prepare];
    [generator selectionChanged];
    generator = nil;
}

@interface TheController ()

@property (strong, nonatomic) EAGLContext *context;
-(void)generateFeedback:(u8)type;

@end

GLint defaultFBO;

bool keys[KEYS_NUM];
StaticArray<Event, MAX_EVENTS_PER_LOOP> events(0);


@implementation TheController

-(void)generateFeedback:(u8)type{
    if ([[UIDevice currentDevice] systemVersion].floatValue < 10.0){
        return;
    }
    printf("Hey %i\n", type);
    switch(type){
        case 0: {
            UISelectionFeedbackGenerator *generator = [[UISelectionFeedbackGenerator alloc] init];
            [generator prepare];
            [generator selectionChanged];
            generator = nil;
        } break;
        case 1: {
            UIImpactFeedbackGenerator *generator = [[UIImpactFeedbackGenerator alloc] initWithStyle:UIImpactFeedbackStyleLight];
            [generator prepare];
            [generator impactOccurred];
            generator = nil;
        } break;
        case 2: {
            UIImpactFeedbackGenerator *generator = [[UIImpactFeedbackGenerator alloc] initWithStyle:UIImpactFeedbackStyleMedium];
            [generator prepare];
            [generator impactOccurred];
            generator = nil;
        } break;
        case 3: {
            UIImpactFeedbackGenerator *generator = [[UIImpactFeedbackGenerator alloc] initWithStyle:UIImpactFeedbackStyleHeavy];
            [generator prepare];
            [generator impactOccurred];
            generator = nil;
        } break;
        case 4: {
            UINotificationFeedbackGenerator *generator = [[UINotificationFeedbackGenerator alloc] init];
            [generator prepare];
            [generator notificationOccurred:UINotificationFeedbackTypeSuccess];
            generator = nil;
        } break;
        case 5: {
            UINotificationFeedbackGenerator *generator = [[UINotificationFeedbackGenerator alloc] init];
            [generator prepare];
            [generator notificationOccurred:UINotificationFeedbackTypeWarning];
            generator = nil;
        } break;
        case 6: {
            UINotificationFeedbackGenerator *generator = [[UINotificationFeedbackGenerator alloc] init];
            [generator prepare];
            [generator notificationOccurred:UINotificationFeedbackTypeError];
            generator = nil;
        } break;
        default:
            break;
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
    
    window_size.x = 2*width;
    window_size.y = 2*height;
    TIME_STEP = 1.f/30.f;
    
    init_basecode();
    init_game();
    
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
    game_loop(keys, events);
    game_draw();
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
            bool hit = false;
            if(point.x < 0.23){
                keys[KEYS_LEFT] = true;
                hit = true;
            }else if(point.x < 0.44){
                keys[KEYS_RIGHT] = true;
                hit = true;
            }else if(point.x > 0.78){
                keys[KEYS_SPACE] = true;
                hit = true;
            }
            if(game_mode == GAME_MODE_PLAY && hit){
                UIImpactFeedbackGenerator *generator = [[UIImpactFeedbackGenerator alloc] initWithStyle:UIImpactFeedbackStyleLight];
                [generator prepare];
                [generator impactOccurred];
                generator = nil;
            }
        }
        touch_point = Vec2(point.x, point.y);
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
        bool kl = keys[KEYS_LEFT], kr = keys[KEYS_RIGHT], ks = keys[KEYS_SPACE];
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
        if(game_mode == GAME_MODE_PLAY){
            if((kl && !keys[KEYS_LEFT]) || (kr && !keys[KEYS_RIGHT]) || (ks && !keys[KEYS_SPACE])){
                UIImpactFeedbackGenerator *generator = [[UIImpactFeedbackGenerator alloc] initWithStyle:UIImpactFeedbackStyleLight];
                [generator prepare];
                [generator impactOccurred];
                generator = nil;
            }
            if((!kl && keys[KEYS_LEFT]) || (!kr && keys[KEYS_RIGHT]) || (!ks && keys[KEYS_SPACE])){
                UISelectionFeedbackGenerator *generator = [[UISelectionFeedbackGenerator alloc] init];
                [generator prepare];
                [generator selectionChanged];
                generator = nil;
            }
        }
        touch_point = Vec2(point.x, point.y);
    }
}

- (void)touchesEnded:(NSSet *)touches withEvent:(UIEvent *)event {
    float height = [[UIScreen mainScreen] bounds].size.height;
    for(UITouch *touch in touches){
        CGPoint previous_point = [touch previousLocationInView:[self view]];
        previous_point.x /= height;
        previous_point.y /= height;
        if(previous_point.y >= 0.78){
            bool hit = false;
            if(previous_point.x < 0.23){
                keys[KEYS_LEFT] = false;
                hit = true;
            }else if(previous_point.x < 0.44){
                keys[KEYS_RIGHT] = false;
                hit = true;
            }else if(previous_point.x > 0.78){
                keys[KEYS_SPACE] = false;
                hit = true;
            }
            if(game_mode == GAME_MODE_PLAY && hit){
                UISelectionFeedbackGenerator *generator = [[UISelectionFeedbackGenerator alloc] init];
                [generator prepare];
                [generator selectionChanged];
                generator = nil;
            }
        }
        if(game_mode == GAME_MODE_MENU)
            select_menu_option();
        touch_point = Vec2(-1.f, -1.f);
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
