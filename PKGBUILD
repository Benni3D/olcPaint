pkgname=olc-paint
pkgver=1.0
pkgrel=1
pkgdesc="A small paint program based on the olcPixelGameEngine"
arch=('any')
url="https://github.com/Benni3D/olcPaint"
license=('custom:OLC-3')
depends=('libpng' 'libx11' 'glibc' 'mesa')
source=("https://github.com/Benni3D/olcPaint/archive/v${pkgver}.tar.gz")
md5sums=('b511120fc47012288a9815fd71870473')
sha256sums=('b6db2349aaf3502aef1b63ca3f8ac66e6403240beb1a8bd58774ee1e46c79998')
sha512sums=('45bbc5ec88f54d618c23d95a250721b7141d45b34cbf845022c1e00c3b2609c467d457bed52644752c6af48bb8dd07caa008f2ea47423908c8b5e4f603e53ba4')

build() {
	cd "${srcdir}/olcPaint-${pkgver}"
	make -j$(nproc)
}

package() {
	cd "${srcdir}/olcPaint-${pkgver}"
	install -m 755 -d "${pkgdir}/usr/bin"
	install -m 755 paint "${pkgdir}/usr/bin/olcPaint"
}