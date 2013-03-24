EAPI=3

EGIT_REPO_URI="https://github.com/eientei/yukkipaste.git"

inherit git-2

DESCRIPTION="Yukkipaste CLI pasting tool"
HOMEPAGE="http://github.com/eientei/yukkipaste"

LICENSE="WTFPLv2"
SLOT="0"

KEYWORDS="amd64 x86"

IUSE="+libmagic"

DEPEND="
	>=dev-util/cmake-2.8
	libmagic? ( sys-apps/file )
"

RDEPEND="
	libmagic? ( sys-apps/file )
"

src_compile() {
	emake || die
}

src_install() {
	emake install DESTDIR="${D}/usr" || die
}

pkg_postinst() {
	elog "Consider using configuration file located in"
	elog "> ${ROOT}usr/share/doc/yukkipaste/config.example"
	elog "to apply it, put it in following location:"
	elog "> ~/.config/yukkipaste/config"
}
