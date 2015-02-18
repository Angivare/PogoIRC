Utilisation d'un compte:
	Afficher/cacher les listes de pseudos/mdp:
		Fenêtres/Liste des pseudos
		Fenêtres/Liste des mots de passe

	Ajouter un pseudo/mdp:
		Options/Nouveau pseudo (CTRL+P)
			Entrer le pseudo
				OU
			Entrer: "pseudo:mdp" où pseudo est le pseudo et mdp est le mot de passe associé au pseudo.
		Options/Nouveau mot de passe
			Entrer le mot de passe
				OU
			Entrer: "alias:mdp" où l'alias est le nom qui sera affiché dans la liste et mdp est le mot de passe qui sera utilisé lorsque cet alias est sélectionné.

	Sélectionner un couple pseudo/mdp:
		Sélectionner dans la liste le pseudo, et le mot de passe par défaut

	Supprimer un pseudo/mdp:
		Double clic sur l'élément dans sa liste

Utilisation des topics:
	Ajouter une nouvelle vue de topic:
		Fenêtres/Nouvelle conversation (CTRL+T)
		Entrer l'url du topic

	Changer l'url du topic sélectionné:
		Options/Changer le topic du dock actuel (CTRL+SHIFT+T)

	Réinitialiser un topic sélectionné:
		Fenêtre/réactualiser le topic (CTRL+R)

Utilisation des forums:
	* PLUS SUPPORTÉ DEPUIS RESPAWN *
	Identique à celle des topics, si ce n'est les raccourcis clavier
	Le rafraîchissement (F5) réactualise tous les forums chargés

Utilisation des topics favoris:
	Sauvegarder l'url du topic sélectionné:
		Favoris/Sauvegarder

	Charger l'url d'un favori dans un nouveau topic:
		Favoris/Charger

Utilisation des thèmes:
	Charger un thème:
		Thèmes/Charger

Options supplémentaires:
Dans config.ini sous [options], ajouter/modifier ces lignes:
	maxPosts=200
	Pour modifier le nombre de posts maximal que peut contenir un topic (lorsque la limite est atteinte, le deuxième post du topic est supprimé pour rajouter le dernier)
	0 signifie qu'il n'y a pas de limite.

	favCount=9
	Pour modifier le nombre de favoris disponibles (au-delà de 9, les raccourcis associés seront improbables)

	pollRate=3000
	Pour modifier le délai de rafraichissement des topics en milliseconde

	forumsPollRate=5000
	Pour modifier le délai de rafraichissement des forums en milliseconde 
	0 signifie aucun rafraîchissement, le seul moyen de les rafraîchir est alors d'utiliser F5

Sous [log], la ligne
	maxLogs=0
	Pour définir le nombre maximal de lignes de logs gardés dans la console.
	Un nombre nul ou négatif signifie pas de limite

Astuces supplémentaires:

	Envoyer un message vide permet de se connecter si ce n'est pas déjà fait.

	Pour ajouter un grand nombre de pseudos dans le logiciel, préférez les ajouter directement dans le fichier de configuration: ouvrez config.ini, allez à la ligne nickAlias= sous [credentials] et ajoutez-y tous vos pseudos séparés par une virgule
	Ajoutez ensuite dans nickData vos pseudos ou vos pseudos et leurs mots de passes associés (sous la forme pseudo:mdp), toujours séparés par une virgule.

	Pour ajouter un grand nombre de mots de passe, c'est la même démarche. passAlias pour l'alias de votre mot de passe (ce qui sera affiché dans l'UI), et passData pour le mot de passe en lui-même