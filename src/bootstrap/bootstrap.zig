const std = @import("std");

pub fn main() !void {
    var gpa = std.heap.GeneralPurposeAllocator(.{  }){};
    var args = try std.process.argsWithAllocator(gpa.allocator());
    var stderr = std.io.getStdErr().writer();

    var argv = std.ArrayList([*:0]const u8).init(gpa.allocator());
    while ( args.next() ) |arg|
        try argv.append( arg );

    var handle = std.DynLib.open( "launcher" ) catch |err| {
        _ = try std.fmt.format( stderr, "Failed to load `launcher` dll: {!}\n", .{ err });
        std.os.exit(1);
    };
    defer handle.close();

    const launcherMain = handle.lookup( *fn(i32, [*]const [*:0]const u8) i32, "LauncherMain" ) orelse {
        _ = try stderr.write( "Failed to load `LauncherMain`, aborting.\n" );
        std.os.exit(1);
    };

    std.os.exit( @intCast( launcherMain( @intCast( argv.items.len ), argv.items.ptr ) ) );
}