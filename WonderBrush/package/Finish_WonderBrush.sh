#!/bin/sh

#strings
NAME="WonderBrush"

# the right place for the Deskbar link
if [ -f /system/servers/locale_server ] && [ -f /system/lib/libzeta.so ]
then
	APPS="/boot/home/config/ZetaMenu/Software/Graphics"
	DOCS="/boot/home/config/ZetaMenu/Documentation"
	MENU="ZETA"
else
	APPS="/boot/home/config/be/YellowBites"
	DOCS=APPS
	MENU="Be"
fi

CREATE="Should a shortcut for $NAME be created in the $MENU menu?"
NO="Skip"
YES="Create Shortcut"

LANGUAGE=$(echo $LC_MESSAGES | sed -e "s/\(.*\)_.*/\1/")

case $LANGUAGE in
	de)	CREATE="Soll eine Verknüpfung zu $NAME im $MENU Menü anlegt werden?"
		NO="Überspringen"
		YES="Verknüpfung anlegen";;
	fr)	CREATE="Souhaitez-vous qu'un raccourci pour $NAME soit crée dans le menu $MENU?"
		NO="Passer"
		YES="Créer le raccourci";;
	ja)	CREATE="$MENU メニューに $NAME のショートカットを作成しますか?"
		NO="スキップ"
		YES="ショートカット作成";;
	nl)	CREATE="Moet er een snelkoppeling voor $NAME in het $MENU menu gemaakt worden?"
		NO="Overslaan"
		YES="Creëer Snelkoppeling";;
	si)	CREATE="Naj se ustvari bližnjica do $NAME v $MENU meniju?"
		NO="Preskoči"
		YES="Ustvari bližnjico";;
	tk)	CREATE="$NAME için $MENU Mönüsüne ipucu kurulsunmu?"
		NO="Geç"
		YES="İpucu etkinleştir";;
	no)	CREATE="Ønsker du at det skal lages en snarvei for $NAME i $MENU menyen?"
		NO="Hopp over"
		YES="Lag snarvei";;

#	)	CREATE=""
#		NO=""
#		YES="";;

#	*)	CREATE="Should a shortcut for $NAME be created in the $MENU menu?"
#		NO="Skip"
#		YES="Create Shortcut";;
esac

LINK="$APPS/$NAME"
DOC_LINK="$DOCS/$NAME"

# ask user wether to create the link
if [ -L "$LINK" ] || [ ! -e "$LINK" ]
then
	ANSWER=`alert --warning "$CREATE" "$NO" "$YES"`
	if [ "$ANSWER" = "$YES" ]
	then
		# app link
		if ! test -d $APPS; then
			mkdir $APPS
		fi
		ln -fs "$PWD/$NAME/$NAME" "$LINK"
		# documentation link
		if test -e "$PWD/$NAME/Dokumentation"; then
			if ! test -d $DOCS; then
				mkdir $DOCS
			fi
			ln -fsn "$PWD/$NAME/Dokumentation" "$DOC_LINK"
		fi
	fi
fi

mimeset -F "$PWD/$NAME/$NAME"

#open the install folder
/system/Tracker $PWD/$NAME

exit 0
