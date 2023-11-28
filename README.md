# ESP32-chatbot-telegram-chatgpt

Ce projet est un bot Telegram hébergé sur ESP32 qui utilise un assistant conversationel d'OpenAI via l'API pour interagir avec les utilisateurs. 

Prérequis

    Créez un bot Telegram à l'aide de BotFather,
    Récupérez son token.
    Créez un compte sur OpenAI,
    Créez un assistant et personalisez le,
    Récupérez son id.

Installation

    Clonez ce dépôt sur votre machine locale.
    Ouvrez le projet dans votre environnement de développement C++ préféré.
    Remplissez les champs à personaliser (ssid, mdp, ...). N'oubliez pas l'id de l'assistant.
    Compilez et téléchargez le code sur votre ESP32.

Utilisation

    Envoyez un message au bot sur Telegram.
    Le bot crée un nouveau thread pour la conversation.
    Puis il ajoute un message au thread.
    Si le bot reçoit une réponse HTTP 400 lors de l'ajout d'un message au thread, il crée un nouveau thread et réessaie d'ajouter le message.
    Il demande à l'assistant de traiter le message.
    Le bot vérifie le statut du thread jusqu'à ce qu'il soit complet, puis récupère les messages du thread et les envoie à l'utilisateur sur Telegram.

Ce que ce projet ne FAIT PAS 

   Attention, ce projet ne prend pas en charge les fonctionnalités avancées des assistants d'OpenAI (envois de fichiers, audios, ...). Il permet juste de faire la conversation avec un assistant préalablement personalisé sur la plateforme OpenAI pour économiser de la place dans l'EPROM de l'ESP. Cela permet aussi d'avoir une conversation suivie sans itérer tous les messages depuis l'ESP.

Contributions

  Les contributions à ce projet sont les bienvenues. 
  Il mérite une bonne factorisation.
  Veuillez soumettre une pull request ou ouvrir une issue pour discuter des modifications que vous souhaitez apporter.

A venir

  Ajout de ChatAction pendant que l'ESP traite les messages.


