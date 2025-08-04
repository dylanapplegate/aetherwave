// Real visual Aetherwave application using native macOS Cocoa frameworks
// This creates an actual window that displays images

#include <iostream>
#include <vector>
#include <string>
#include <filesystem>

#ifdef __APPLE__
#import <Cocoa/Cocoa.h>
#import <QuartzCore/QuartzCore.h>

@interface AetherwaveWindow : NSWindow
@end

@interface AetherwaveViewController : NSViewController
@property (strong) NSImageView *imageView;
@property (strong) NSTextField *statusLabel;
@property (strong) NSTextField *controlsLabel;
@property (assign) NSInteger currentImageIndex;
@property (strong) NSMutableArray<NSString*> *imagePaths;
- (void)loadImagesFromDirectory:(NSString*)path;
- (void)showCurrentImage;
- (void)nextImage;
- (void)previousImage;
@end

@implementation AetherwaveViewController

- (void)viewDidLoad {
    [super viewDidLoad];

    self.imagePaths = [[NSMutableArray alloc] init];
    self.currentImageIndex = 0;

    // Create main image view
    self.imageView = [[NSImageView alloc] init];
    self.imageView.imageScaling = NSImageScaleProportionallyUpOrDown;
    self.imageView.imageAlignment = NSImageAlignCenter;
    self.imageView.translatesAutoresizingMaskIntoConstraints = NO;
    self.imageView.wantsLayer = YES;
    self.imageView.layer.backgroundColor = [[NSColor blackColor] CGColor];

    // Create status label
    self.statusLabel = [[NSTextField alloc] init];
    self.statusLabel.stringValue = @"🌊 Aetherwave Display Engine v2.0 - Loading...";
    self.statusLabel.backgroundColor = [NSColor clearColor];
    self.statusLabel.textColor = [NSColor whiteColor];
    self.statusLabel.font = [NSFont systemFontOfSize:16 weight:NSFontWeightMedium];
    self.statusLabel.alignment = NSTextAlignmentCenter;
    self.statusLabel.bezeled = NO;
    self.statusLabel.editable = NO;
    self.statusLabel.selectable = NO;
    self.statusLabel.translatesAutoresizingMaskIntoConstraints = NO;

    // Create controls label
    self.controlsLabel = [[NSTextField alloc] init];
    self.controlsLabel.stringValue = @"SPACE/→: Next  •  ←: Previous  •  ESC: Exit";
    self.controlsLabel.backgroundColor = [NSColor clearColor];
    self.controlsLabel.textColor = [NSColor lightGrayColor];
    self.controlsLabel.font = [NSFont systemFontOfSize:12];
    self.controlsLabel.alignment = NSTextAlignmentCenter;
    self.controlsLabel.bezeled = NO;
    self.controlsLabel.editable = NO;
    self.controlsLabel.selectable = NO;
    self.controlsLabel.translatesAutoresizingMaskIntoConstraints = NO;

    // Add subviews
    [self.view addSubview:self.imageView];
    [self.view addSubview:self.statusLabel];
    [self.view addSubview:self.controlsLabel];

    // Set up constraints
    [NSLayoutConstraint activateConstraints:@[
        // Image view - full screen except for labels
        [self.imageView.topAnchor constraintEqualToAnchor:self.view.topAnchor constant:50],
        [self.imageView.leadingAnchor constraintEqualToAnchor:self.view.leadingAnchor],
        [self.imageView.trailingAnchor constraintEqualToAnchor:self.view.trailingAnchor],
        [self.imageView.bottomAnchor constraintEqualToAnchor:self.view.bottomAnchor constant:-50],

        // Status label - top
        [self.statusLabel.topAnchor constraintEqualToAnchor:self.view.topAnchor constant:15],
        [self.statusLabel.centerXAnchor constraintEqualToAnchor:self.view.centerXAnchor],
        [self.statusLabel.widthAnchor constraintEqualToConstant:600],

        // Controls label - bottom
        [self.controlsLabel.bottomAnchor constraintEqualToAnchor:self.view.bottomAnchor constant:-15],
        [self.controlsLabel.centerXAnchor constraintEqualToAnchor:self.view.centerXAnchor],
        [self.controlsLabel.widthAnchor constraintEqualToConstant:400],
    ]];

    // Set background color
    self.view.wantsLayer = YES;
    self.view.layer.backgroundColor = [[NSColor blackColor] CGColor];

    // Load images
    [self loadImagesFromDirectory:@"assets/images"];

    if (self.imagePaths.count > 0) {
        [self showCurrentImage];
    } else {
        self.statusLabel.stringValue = @"No images found. Add images to assets/images/ directory";

        // Show a placeholder
        NSImage *placeholderImage = [NSImage imageWithSize:NSMakeSize(400, 300) flipped:NO drawingHandler:^BOOL(NSRect dstRect) {
            [[NSColor darkGrayColor] setFill];
            NSRectFill(dstRect);

            NSDictionary *attributes = @{
                NSFontAttributeName: [NSFont systemFontOfSize:24],
                NSForegroundColorAttributeName: [NSColor lightGrayColor]
            };
            NSString *text = @"🖼️\nAdd images to\nassets/images/";
            NSSize textSize = [text sizeWithAttributes:attributes];
            NSPoint textPoint = NSMakePoint((dstRect.size.width - textSize.width) / 2,
                                          (dstRect.size.height - textSize.height) / 2);
            [text drawAtPoint:textPoint withAttributes:attributes];

            return YES;
        }];

        self.imageView.image = placeholderImage;
    }
}

- (void)loadImagesFromDirectory:(NSString*)path {
    [self.imagePaths removeAllObjects];

    NSFileManager *fileManager = [NSFileManager defaultManager];
    NSError *error;
    NSArray *files = [fileManager contentsOfDirectoryAtPath:path error:&error];

    if (error) {
        NSLog(@"Error reading directory %@: %@", path, error.localizedDescription);
        // Try to create the directory
        [fileManager createDirectoryAtPath:path withIntermediateDirectories:YES attributes:nil error:nil];
        return;
    }

    NSArray *imageExtensions = @[@"jpg", @"jpeg", @"png", @"gif", @"bmp", @"tiff", @"tif"];

    for (NSString *file in files) {
        NSString *extension = [[file pathExtension] lowercaseString];
        if ([imageExtensions containsObject:extension]) {
            NSString *fullPath = [path stringByAppendingPathComponent:file];
            [self.imagePaths addObject:fullPath];
        }
    }

    NSLog(@"Found %lu images in %@", (unsigned long)self.imagePaths.count, path);
}

- (void)showCurrentImage {
    if (self.imagePaths.count == 0) return;

    NSString *imagePath = self.imagePaths[self.currentImageIndex];
    NSImage *image = [[NSImage alloc] initWithContentsOfFile:imagePath];

    if (image) {
        self.imageView.image = image;

        NSString *filename = [imagePath lastPathComponent];
        NSString *status = [NSString stringWithFormat:@"🎨 %@ (%ld of %lu)",
                           filename,
                           (long)(self.currentImageIndex + 1),
                           (unsigned long)self.imagePaths.count];
        self.statusLabel.stringValue = status;

        NSLog(@"Displaying: %@", filename);
    } else {
        NSLog(@"Failed to load image: %@", imagePath);
    }
}

- (void)nextImage {
    if (self.imagePaths.count == 0) return;

    self.currentImageIndex = (self.currentImageIndex + 1) % self.imagePaths.count;
    [self showCurrentImage];
}

- (void)previousImage {
    if (self.imagePaths.count == 0) return;

    self.currentImageIndex = (self.currentImageIndex == 0) ?
        self.imagePaths.count - 1 : self.currentImageIndex - 1;
    [self showCurrentImage];
}

@end

@implementation AetherwaveWindow

- (void)keyDown:(NSEvent *)event {
    NSString *characters = [event characters];
    if ([characters length] > 0) {
        unichar character = [characters characterAtIndex:0];

        AetherwaveViewController *viewController = (AetherwaveViewController*)self.contentViewController;

        switch (character) {
            case NSRightArrowFunctionKey:
            case ' ':
                [viewController nextImage];
                break;

            case NSLeftArrowFunctionKey:
                [viewController previousImage];
                break;

            case 27: // ESC
                [NSApp terminate:self];
                break;

            default:
                [super keyDown:event];
                break;
        }
    }
}

- (BOOL)canBecomeKeyWindow {
    return YES;
}

@end

@interface AetherwaveAppDelegate : NSObject <NSApplicationDelegate>
@property (strong) AetherwaveWindow *window;
@end

@implementation AetherwaveAppDelegate

- (void)applicationDidFinishLaunching:(NSNotification *)notification {
    // Create main window
    NSRect frame = NSMakeRect(100, 100, 1200, 800);
    self.window = [[AetherwaveWindow alloc] initWithContentRect:frame
                                                     styleMask:NSWindowStyleMaskTitled |
                                                               NSWindowStyleMaskClosable |
                                                               NSWindowStyleMaskMiniaturizable |
                                                               NSWindowStyleMaskResizable
                                                       backing:NSBackingStoreBuffered
                                                         defer:NO];

    self.window.title = @"Aetherwave Display Engine";
    self.window.backgroundColor = [NSColor blackColor];

    // Create and set view controller
    AetherwaveViewController *viewController = [[AetherwaveViewController alloc] init];
    self.window.contentViewController = viewController;

    // Show window
    [self.window makeKeyAndOrderFront:nil];
    [self.window center];

    // Make app active
    [NSApp activateIgnoringOtherApps:YES];

    NSLog(@"Aetherwave Display Engine started");
}

- (BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication *)sender {
    return YES;
}

@end

#endif

namespace Aetherwave {

class VisualApp {
public:
    void run() {
#ifdef __APPLE__
        NSLog(@"🌊 Starting Aetherwave Visual Display Engine...");

        @autoreleasepool {
            NSApplication *app = [NSApplication sharedApplication];
            AetherwaveAppDelegate *delegate = [[AetherwaveAppDelegate alloc] init];
            [app setDelegate:delegate];
            [app run];
        }
#else
        std::cout << "Visual mode currently only supported on macOS" << std::endl;
        std::cout << "Falling back to terminal mode..." << std::endl;
        // Could add Linux/Windows visual implementation here
#endif
    }
};

} // namespace Aetherwave

int main() {
    std::cout << "🌊 Aetherwave Display Engine v2.0" << std::endl;
    std::cout << "📱 Starting visual application..." << std::endl;

    try {
        Aetherwave::VisualApp app;
        app.run();
    } catch (const std::exception& e) {
        std::cerr << "❌ Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
