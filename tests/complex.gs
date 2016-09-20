using core
using math

# Definition of a complex number
type Complex(realp:float, imagp:float) {
    @Getter
    let real = realp

    @Getter
    let imag = imagp

    func add(other:Complex) -> Complex {
        return Complex(real + other.getReal(), imag + other.getImag())
    }

    func sub(other:Complex) -> Complex {
        return Complex(real - other.getReal(), imag - other.getImag())
    }

    func mul(other:Complex) -> Complex {
        let c_real = real * other.getReal() - imag * other.getImag()
        let c_imag = imag * other.getReal() + real * other.getImag()
        return Complex(c_real, c_imag)
    }

    func div(other:Complex) -> Complex {
        let tmp1 = real * other.getReal() + imag * other.getImag()
        let tmp2 = imag * other.getReal() - real * other.getImag()
        let tmp3 = other.getReal() * other.getReal() + other.getImag() * other.getImag()
        return Complex(tmp1/tmp3, tmp2/tmp3)
    }

    # Factor multiplication: this * Complex(factor, 0.0)
    func fmul(factor:float) -> Complex {
        return Complex(real * factor, imag * factor)
    }

    # Complex conjugate
    func conj() -> Complex {
        return Complex(real, -imag)
    }

    # Complex absolute value (length)
    func cabs() -> float {
        return sqrt(real * real + imag * imag)
    }

    # Complex exponent function
    func cexp() -> Complex {
        return Complex(exp(real) * cos(imag), exp(real) * sin(imag))
    }

    # Complex sine
    func csin() -> Complex {
        return Complex(cosh(imag) * sin(real), sinh(imag) * cos(real))
    }

    func to_str() -> char[] {
        return "$real + i$imag"
    }
}

# Imaginary unit
let i = Complex(0.0, 1.0)
