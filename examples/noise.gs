# Perlin-noise in Golem.
# Original version by Ken Perlin.
# Ported from Rosetta Code Go-example.
# @author Alexander Koch 2015
using core
using math

let permutation = [
	151, 160, 137, 91, 90, 15, 131, 13, 201, 95, 96, 53, 194, 233, 7, 225,
    140, 36, 103, 30, 69, 142, 8, 99, 37, 240, 21, 10, 23, 190, 6, 148,
    247, 120, 234, 75, 0, 26, 197, 62, 94, 252, 219, 203, 117, 35, 11, 32,
    57, 177, 33, 88, 237, 149, 56, 87, 174, 20, 125, 136, 171, 168, 68, 175,
    74, 165, 71, 134, 139, 48, 27, 166, 77, 146, 158, 231, 83, 111, 229, 122,
    60, 211, 133, 230, 220, 105, 92, 41, 55, 46, 245, 40, 244, 102, 143, 54,
    65, 25, 63, 161, 1, 216, 80, 73, 209, 76, 132, 187, 208, 89, 18, 169,
    200, 196, 135, 130, 116, 188, 159, 86, 164, 100, 109, 198, 173, 186, 3, 64,
    52, 217, 226, 250, 124, 123, 5, 202, 38, 147, 118, 126, 255, 82, 85, 212,
    207, 206, 59, 227, 47, 16, 58, 17, 182, 189, 28, 42, 223, 183, 170, 213,
    119, 248, 152, 2, 44, 154, 163, 70, 221, 153, 101, 155, 167, 43, 172, 9,
    129, 22, 39, 253, 19, 98, 108, 110, 79, 113, 224, 232, 178, 185, 112, 104,
    218, 246, 97, 228, 251, 34, 242, 193, 238, 210, 144, 12, 191, 179, 162, 241,
    81, 51, 145, 235, 249, 14, 239, 107, 49, 192, 214, 31, 181, 199, 106, 157,
    184, 84, 204, 176, 115, 121, 50, 45, 127, 4, 150, 254, 138, 236, 205, 93,
    222, 114, 67, 29, 24, 72, 243, 141, 128, 195, 78, 66, 215, 61, 156, 180]

let p = permutation.append(permutation)

func fade(t: float) -> float {
	return t * t * t * (t * (t * 6.0 - 15.0) + 10.0)
}

func lerp(t: float, a: float, b: float) -> float {
	return a + t * (b-a)
}

func grad(hash: int, x: float, y: float, z: float) -> float {
	let h = hash & 15

    if h = 0 || h = 12 {
        return x + y
	} else if h = 1 || h = 14 {
        return y - x
	} else if h = 2 {
        return x - y
	} else if h = 3 {
        return -x - y
	} else if h = 4 {
        return x + z
	} else if h = 5 {
        return z - x
	} else if h = 6 {
        return x - z
	} else if h = 7 {
        return -x - z
	} else if h = 8 {
        return y + z
    } else if h = 9 || h = 13 {
        return z - y
	} else if h = 10 {
        return y - z
    } else {
    	return -y - z
	}
	return 0.0
}

func noise(mut px: float, mut py: float, mut pz: float) -> float {
	let X = floor(px).to_i() & 255
	let Y = floor(py).to_i() & 255
	let Z = floor(pz).to_i() & 255

	let x = px - floor(px)
	let y = py - floor(py)
	let z = pz - floor(pz)

	let u = fade(x)
	let v = fade(y)
	let w = fade(z)

	let A = p[X] + Y
	let AA = p[A] + Z
	let AB = p[A+1] + Z
	let B = p[X+1] + Y
	let BA = p[B] + Z
	let BB = p[B+1] + Z

	let g1 = grad(p[AA], x, y, z)
	let g2 = grad(p[BA], x-1.0, y, z)
	let g3 = grad(p[AB], x, y-1.0, z)
	let g4 = grad(p[BB], x-1.0, y-1.0, z)
	let g5 = grad(p[AA+1], x, y, z-1.0)
	let g6 = grad(p[BA+1], x-1.0, y, z-1.0)
	let g7 = grad(p[AB+1], x, y-1.0, z-1.0)
	let g8 = grad(p[BB+1], x-1.0, y-1.0, z-1.0)

	let l1 = lerp(u, g1, g2)
	let l2 = lerp(u, g3, g4)
	let l3 = lerp(u, g5, g6)
	let l4 = lerp(u, g7, g8)

	return lerp(w, lerp(v, l1, l2), lerp(v, l3, l4))
}

let res = noise(3.14, 42.0, 7.0)
println(res)
