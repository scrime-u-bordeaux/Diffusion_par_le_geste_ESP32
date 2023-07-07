# Diffusion_par_le_geste_ESP32

mixage acousmonium, projet de Jean Michel Rivet, orchestration de haut-parleurs.  

## Matériel utilisé : 

- Micro-contrôleur : ESP32 DFRobot firebeetle (datasheet : https://wiki.dfrobot.com/FireBeetle_ESP32_IOT_Microcontroller%28V3.0%29__Supports_Wi-Fi_&_Bluetooth__SKU__DFR0478)
- Accéléromètre 3 axes et Gyroscope 3 axes : MPU6050
- capteur et récepteur infrarouge 

## Bibliothèques utilisées : 
### Celles pour les ESP :

- IRremote (arduino officielle V4.1.2)
- WebSockets (Markus Settler V2.4.0)
- Adafruit MPU6050 (Adafruit V2.4.4)
- Adafruit SSD1306 (Adafruit V2.5.7)
- Adafruit GFX Library (Adafruit V1.11.5)
- Adafruit BusIO (Adafruit V1.14.1)
- Adafruit Unified Sensor (Adafruit V1.1.9)

### Celles pour le script node.js : 
- ws : https://github.com/websockets/ws
- node-osc : https://www.npmjs.com/package/node-osc

## Comment cela fonctionne ? 
### Côté micro-controleurs 

Les micro-controleurs ESP32 sont divisés en deux catégories : ceux liés aux enceintes, ceux liés aux télécommandes. 
Les télécommandes possèdent plusieurs boutons, un émetteur infrarouge et un mpu (accéléromètre + gyroscope). Quant aux ESP liés aux enceintes, ils sont tous connectés à un récepteur infrarouge, ainsi qu'à une led verte et une rouge. 

Actuellement, la télécommande possède trois boutons : un pour la sélection d'enceinte, un pour la déselection d'enceinte et un pour récupérer les valeurs du mpu.

Lorsque les boutons de sélection et de déselection sont activés, ils envoient un message en infrarouge aux cartes des enceintes. Une fois reçu, la carte transmet le message au serveur websocket, et allume certaines leds selont le message reçu. 
Lorsque le bouton pour récupérer les données du mpu est activé, elles sont directement envoyées au script javascript. 

### Côté script javascript 

C'est lui qui crée le serveur websocket et qui envoie les valeurs à ossia.score via OSC. 
Les messages échangés entre carte esp et serveur sont toutes de type **string**. Pour récupérer les valeurs souhaitées, les messages sont construits comme suit : "AddressMAC\nNomdelévènementn°1\nValeurdelévènementn°1\nNomdelévènementn°2..." Il peut y avoir plusieurs évènements envoyés dans le même message. 
Ainsi on sépare la chaîne de caractère avec comme délimiteur \n. et selon l'adresse MAC et le nom de l'évènement, différentes fonctions sont appelées (voir le dernier paragraphe pour plus de détail).

## Mise en fonction :

    1°) Allumer les cartes ESP, via un secteur et un câble USB ou avec une batterie si c'est possible. 
Les deux leds des ESP liées aux hauts-parleurs seront allumées tant que les cartes ne seront pas connectées au Wifi indiquée dans leur firmware. 
En ce qui concerne la télécommande, la led blinkera tant que la télécommande ne sera pas connectée au Wifi. 
Lorsqu'une carte est connectée, peu importe laquelle, la led bleue de la carte reste en surbrilliance tant que la carte associée n'est pas connectée au serveur websocket. 
    2°) Connecter le pc au même Wifi que celle indiquée dans le firmware des cartes et changer l'IPv4 si besoin. 
    3°) Lancer le script .mjs avec node ce qui lancera le serveur websocket, les leds bleues de toutes les cartes devraient s'éteindre, et un message sur la console devrait s'afficher chaque fois qu'une carte se connecte. 
    4°) Dans ossia.score, ne pas oublier de faire "learn" si jamais le processus OSC vient d'être créé. Sinon il n'y a qu'à lancer le son et jouer avec les boutons.

## Explications des fonctions javascript  :

Dans ```relationMacAddressOscAddress.js``` se trouve deux tableaux contenant respectivement les adresses MAC des cartes liées aux enceintes et celles liées aux télécommandes. Deux tableaux sont créés en lien avec celui regroupant les adresses MAC des cartes-enceintes, un contenant des booléens pour savoir si la carte-enceinte est sélectionnée ou non et un un autre contenant des floats, la valeur envoyée à ossia.score. 

Lorsqu'un message en websocket est reçu, la fonction ```receiving data ```est la première appelée. Grâce au délimiteur "\n", est récupérée l'adresse mac et différents appels de fonction sont faits selon si c'est l'adresse mac d'une carte-enceinte ou d'une carte-télécommande. 

La fonction ``interprateHpData`` actualise le tableau de sélection des enceintes. 

La fonction ``volumeButtonPressed`` calcule selon les valeurs reçues du mpu la nouvelle valeur a envoyer à ossia.score et actualise le tableau de valeurs des différentes carte-enceinte selon si elles sont sélectionnées ou non.  

La nouvelle valeur du volume pour les cartes-enceintes sélectionées est calculée de la manière suivante : 
    - les valeurs reçues sont la valeur du gyroscope sur l'axe des x en **w/s** et le temps d'actionnage du bouton en **ms**. 
    - L'intervalle des valeurs du gyroscope va de -500 w/ms à 500 w/ms. Sur ossia.score, les décibels vont de -70 à 4.
    - On récupère la valeur **w** dont on prend le pourçentage via la valeur max (en valeur absolue) possible pour le gyroscope. On récupère alors une nouvelle valeur de volume pour les enceintes en multipliant ce pourçentage par 74. 