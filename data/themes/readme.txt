Instructions pour la création d'un thème:

Un thème est contenu à l'intérieur d'un dossier dans le dossier themes.
Il contient au moins 5 fichiers:
	-header.html
	-body.html
	-footer.html
	-notif.wav
	-style.css
Et un dossier img contenant au moins un fichier:
	-notif.png

Commençons par l'utilisation des fichiers les plus basiques:
	
	notif.wav:
Si l'utilisateur a activé les notifications, notif.wav est joué à chaque fois qu'un nouveau message est reçu. notif.wav peut être un son vide pour que le thème ne joue jamais de son de notification.

	-img/notif.png
Lorsqu'un topic contient un message non lu, img/notif.png est dessiné dans sa barre de titre, très exactement au coin supérieur gauche.

Ensuite viennent les fichiers plus complexes. Pour modifier ceux-là, une compréhension (ou une aide de quelqu'un qui comprend) du html et du css est nécessaire.

	-style.css
Ce fichier de style est chargé au démarrage de l'application et appliqué à l'ensemble de l'application. Il utilise un langage similaire au css appelé le qss (Qt Style Sheet) dont voici la documentation: http://qt-project.org/doc/qt-4.8/stylesheet-reference.html
Il permet globalement de modifier la couleur de toute l'application excepté les topics. Il peut également permettre de modifier un peu la position de certains éléments qui peuvent accepter des attributs margin/padding, ou modifier la taille des éléments qui acceptent les attributs height/min-height, etc..

Pour les fichiers suivants, il est nécessaire de comprendre comment fonctionne une vue de topic:
Une vue de topic est un élément qui contient un moteur web chargeant une page html générée dynamiquement par pogoIRC. Il est alors possible de le personnaliser de manière avancée en utilisant du html, css ou même javascript.

	-header.html & footer.html
Le contenu de ces deux fichiers sont concaténés et ensuite chargés dans le moteur web initial de la page.
Ils contiennent donc les éléments statiques de votre topic: les feuilles de style css, les scripts js, ...
Avant le chargement, toutes les occurences de $DIR$ sont remplacés par le chemin de la racine de pogoIRC (le dossier où est contenu le fichier pogoIRC.exe) pour faciliter le placement de liens relatifs.
/!\ Attention! Vous devriez absolument laisser la balise app-contents, car elle est utilisée par l'application pour savoir où doivent être ajoutés les posts /!\

	-body.html
A chaque fois qu'un nouveau post est détecté par pogoIRC, pogoIRC récupère le contenu de ce fichier, remplace certains mots-clefs par les valeurs correspondantes et envoie le texte ainsi généré à la fonction addPost() définie dans $DIR$/data/common.js

les mots-clefs remplacés par pogoIRC sont les suivants:
	$ID$
	$TIME$
	$DATE$
	$RANK$
	$AVATAR$
		pour la rétrocompatibilité, ces deux-là équivalent à $AVATAR$ :
		$AVATAR_SMALL$
		$AVATAR_LARGE$
	$NICK$
	$LWNICK$ (pseudo en minuscules)
	$EDITED$
	$EDITTIME$
	$EDITDATE$
	$EDITNICK$
	$SIGN$
	$MSG$

Par défaut, addPost() rajoute l'élément div de classe app-post0 ou app-post1 dans la balise app-contents à chaque fois qu'un post est ajouté.
Il ajoute ensuite dans app-post le code html qui lui est fourni en argument.

Il y a maintenant également une fonction refresh() dans $DIR$/data/common.js, qui est appelée à chaque rafraîchissement du topic, avec les arguments suivants dans l'ordre:
	ID du forum
	ID du topic
	n° de la page
	Nombre de connectés
	Nombre de messages privés

Pour une personnalisation encore plus avancée, vous pouvez également modifier common.js, à vos risques et périls.