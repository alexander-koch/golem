# Sample three dimensional vector implementation
using core
using math

type Vec3(pX:float, pY:float, pZ:float) {
    @Getter
    let x = pX
    @Getter
    let y = pY
    @Getter
    let z = pZ

	func add(other:Vec3) -> Vec3 {
		return Vec3(x + other.getX(), y + other.getY(), z + other.getZ())
	}

	func sub(other:Vec3) -> Vec3 {
        return Vec3(x - other.getX(), y - other.getY(), z - other.getZ())
	}

	func dot(other:Vec3) -> float {
		return x * other.getX() + y * other.getY() + z * other.getZ()
	}

	func cross(other:Vec3) -> Vec3 {
		let nx = y * other.getZ() - z * other.getY()
		let ny = z * other.getX() - x * other.getZ()
		let nz = x * other.getY() - y * other.getX()
		return Vec3(nx, ny, nz)
	}

    func length() -> float {
        return sqrt(x*x + y*y + z*z)
    }

    func normalize() -> Vec3 {
        let len = length()
        return Vec3(x / len, y / len, z / len)
    }

	func toArray() -> float[] {
		return [x,y,z]
	}
}
