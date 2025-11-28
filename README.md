# Zenoh ROS2DDS Bridge Example

Minimal implementation for communication between Zenoh and ROS2 DDS, supporting Python / Rust / C++.

## Installation

```bash
# Bridge (ROS2 machine)
curl -L https://download.eclipse.org/zenoh/debian-repo/zenoh-public-key | sudo gpg --dearmor -o /etc/apt/keyrings/zenoh-public-key.gpg
echo "deb [signed-by=/etc/apt/keyrings/zenoh-public-key.gpg] https://download.eclipse.org/zenoh/debian-repo/ /" | sudo tee -a /etc/apt/sources.list
sudo apt update && sudo apt install zenoh-bridge-ros2dds

# Python
pip install -r python/requirements.txt

# Rust
cargo build --release --manifest-path rust/Cargo.toml

# C++
cmake -S cpp -B cpp/build && cmake --build cpp/build
```

## Usage

### Start Zenoh Bridge

```bash
# Terminal 1: Start Zenoh Bridge
export ROS_LOCALHOST_ONLY=1                     # Foxy/Humble
export ROS_AUTOMATIC_DISCOVERY_RANGE=LOCALHOST  # Iron+
export RMW_IMPLEMENTATION=rmw_cyclonedds_cpp
zenoh-bridge-ros2dds
```

### Test 1: Zenoh Publisher → ROS2 Subscriber

```bash
# Terminal 2: Start ROS2 subscriber
export ROS_LOCALHOST_ONLY=1                     # Foxy/Humble
export ROS_AUTOMATIC_DISCOVERY_RANGE=LOCALHOST  # Iron+
export RMW_IMPLEMENTATION=rmw_cyclonedds_cpp
ros2 topic echo /cmd_vel geometry_msgs/msg/Twist

# Terminal 3: Start Zenoh publisher
python python/publisher.py -b localhost:7447
./rust/target/release/publisher localhost:7447
./cpp/build/publisher localhost:7447
```

**Note**: For Zenoh → ROS2 direction, you must start the ROS2 subscriber first, otherwise the Bridge will not forward messages.

### Test 2: ROS2 Publisher → Zenoh Subscriber

```bash
# Terminal 2: Start Zenoh subscriber
python python/subscriber.py -b localhost:7447
./rust/target/release/subscriber localhost:7447
./cpp/build/subscriber localhost:7447

# Terminal 3: Start ROS2 publisher
export ROS_LOCALHOST_ONLY=1                     # Foxy/Humble
export ROS_AUTOMATIC_DISCOVERY_RANGE=LOCALHOST  # Iron+
export RMW_IMPLEMENTATION=rmw_cyclonedds_cpp
ros2 topic pub /cmd_vel geometry_msgs/msg/Twist "{linear: {x: 1.0, y: 0.0, z: 0.0}, angular: {x: 0.0, y: 0.0, z: 0.5}}" --once
```

### Test 3: ROS2 Server ↔ Zenoh Client

**Note**: ROS2 services must be started before they can be discovered. If you start the client before the server, please restart the bridge.

```bash
# Terminal 1: Start Bridge
export ROS_LOCALHOST_ONLY=1                     # Foxy/Humble
export ROS_AUTOMATIC_DISCOVERY_RANGE=LOCALHOST  # Iron+
export RMW_IMPLEMENTATION=rmw_cyclonedds_cpp
zenoh-bridge-ros2dds

# Terminal 2: Start ROS2 Server on ROS2 machine
export ROS_LOCALHOST_ONLY=1                     # Foxy/Humble
export ROS_AUTOMATIC_DISCOVERY_RANGE=LOCALHOST  # Iron+
export RMW_IMPLEMENTATION=rmw_cyclonedds_cpp
ros2 run demo_nodes_cpp add_two_ints_server

# Terminal 3: Call from remote machine (Client)
python python/client.py -b localhost:7447
./rust/target/release/client localhost:7447
./cpp/build/client localhost:7447
```

### Test 4: Zenoh Server ↔ ROS2 Client

```bash
# Terminal 1: Start Bridge
export ROS_LOCALHOST_ONLY=1                     # Foxy/Humble
export ROS_AUTOMATIC_DISCOVERY_RANGE=LOCALHOST  # Iron+
export RMW_IMPLEMENTATION=rmw_cyclonedds_cpp
zenoh-bridge-ros2dds

# Terminal 2: Start Server on remote machine
python python/server.py -b localhost:7447
./rust/target/release/server localhost:7447
./cpp/build/server localhost:7447

# Terminal 3: Call Service on ROS2 machine
export ROS_LOCALHOST_ONLY=1                     # Foxy/Humble
export ROS_AUTOMATIC_DISCOVERY_RANGE=LOCALHOST  # Iron+
export RMW_IMPLEMENTATION=rmw_cyclonedds_cpp
ros2 service call /add_two_ints example_interfaces/srv/AddTwoInts "{a: 3, b: 5}"
```

## CDR Serialization

Supports all ROS2 message types.

### Python

```python
from pycdr2 import IdlStruct
from pycdr2.types import float64
from dataclasses import dataclass

@dataclass
class Vector3(IdlStruct):
    x: float64
    y: float64
    z: float64

@dataclass
class Twist(IdlStruct):
    linear: Vector3
    angular: Vector3

# Create
twist = Twist(linear=Vector3(x=0.5, y=0.0, z=0.0), angular=Vector3(x=0.0, y=0.0, z=0.2))

# Serialize
data = twist.serialize()

# Deserialize
twist = Twist.deserialize(data)
```

### Rust

```rust
use serde::{Serialize, Deserialize};

#[derive(Serialize, Deserialize)]
struct Vector3 { x: f64, y: f64, z: f64 }

#[derive(Serialize, Deserialize)]
struct Twist { linear: Vector3, angular: Vector3 }

// Create
let twist = Twist {
    linear: Vector3 { x: 0.5, y: 0.0, z: 0.0 },
    angular: Vector3 { x: 0.0, y: 0.0, z: 0.2 },
};

// Serialize
let data = cdr::serialize(&twist).unwrap();

// Deserialize
let twist: Twist = cdr::deserialize(&data).unwrap();
```

### C++

```cpp
// Define struct
struct Vector3 { double x, y, z; };
struct Twist { Vector3 linear, angular; };

// Create
Twist twist{{0.5, 0, 0}, {0, 0, 0.2}};

// Serialize
auto data = cdr::serialize(twist);

// Deserialize
Twist twist2;
cdr::deserialize(data.data(), data.size(), twist2);
```

## Known Issues

### ROS2 → Zenoh Direction: Resources Not Cleaned Up After Subscriber/Server Reconnection

**Phenomenon**: After Zenoh Subscriber or Server restarts, resources on the Bridge side are not cleaned up, resulting in duplicate messages/requests

**Cause**: zenoh-bridge-ros2dds v1.6.2 bug, DDS resources (subscription/queryable) created by Bridge are not cleaned up when Zenoh side disconnects

**Temporary Solution**: Restart Bridge before running Zenoh subscriber or server each time

**Issue**: https://github.com/eclipse-zenoh/zenoh-plugin-ros2dds/issues/570

## Development

### Setup pre-commit hooks

```bash
pip install pre-commit
pre-commit install
```

### Format all code

```bash
pre-commit run --all-files
```
