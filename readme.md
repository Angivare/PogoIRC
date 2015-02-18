PogoIRC, le logiciel permettant d'afficher un topic de JVC en live.

Les fichiers dans ./data/ sont fournis tels quels à l'utilisateur(ils ne nécessitent aucune compilation, ou autre).
Le dossier contient différentes ressources: les thèmes chargés par la QWebView, le script se chargeant de l'ajout de messages, et les dictionnaires de correction orthographique.
Toutes les autres données (hors config.ini) devraient être stockées ici.

Le correcteur orthographique repose sur hunspell: il va falloir, si vous voulez compiler le projet, compiler d'abord hunspell (ou alors supprimer la classe SpellChecker et le code y faisant référence dans TextEdit)

Si vous voulez ajouter du code spécifique à une plateforme (un fix pour que le logiciel fonctionne comme il faut sur MAC par exemple), utilisez les macros prédéfinies par Qt:

```
#ifdef Q_OS_WIN32
	const QString url("http://irc.pogo.angiva.re/winpatcher");
#elif defined(Q_OS_LINUX)
	const QString url("http://irc.pogo.angiva.re/linpatcher");
#elif defined(Q_OS_MAC)
	const QString url("http://irc.pogo.angiva.re/macpatcher");
#endif
```

Et veuillez S.V.P utilisez des _tabulations_ plutôt que des _espaces_