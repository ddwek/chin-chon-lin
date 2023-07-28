CHIN_CHON_LIN_DATADIR = ${prefix}/share/chin-chon-lin/

AM_CXXFLAGS = `pkg-config --cflags gtk+-3.0` `pkg-config --cflags librsvg-2.0` -std=c++11 -O2
AM_CXXFLAGS += -DCHIN_CHON_LIN_DATADIR=\"${CHIN_CHON_LIN_DATADIR}\"
AM_CXXFLAGS += -DDATAROOTDIR=\"${datarootdir}\"
AM_CXXFLAGS += -DLOCALEDIR=\"${datarootdir}/locale\"

bin_PROGRAMS = chin-chon-lin
chin_chon_lin_SOURCES = \
			Cmdline.cc \
			UserInterface.cc \
			Menu.cc \
			Logic.cc \
			Board.cc \
			Player.cc \
			Tile.cc \
			Stack.cc \
			Deck.cc \
			Card.cc \
			main.cc

chin_chon_lin_LDADD = @LIBINTL@
chin_chon_lin_LDFLAGS = `pkg-config --libs gtk+-3.0` `pkg-config --libs librsvg-2.0`
