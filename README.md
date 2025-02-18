### M9 UF2 PAC5

## Integrants

- Xavier Far Martinez 
- Ruben Rodriguez Lopez
- Oscar Mur Matutano

### Pedra, Paper, Tisores, lagartox, spock. Multijugador, en Temps Real i amb UDP

## Funcionament del Servidor

1. **Inicialització**: El servidor inicialitza la llibreria Winsock i crea un socket per escoltar connexions entrants.
2. **Escolta de Connexions**: El servidor escolta connexions entrants en el port especificat (per defecte, 9000).
3. **Gestió de Jugadors**: Quan un client es connecta, es crea un nou objecte `Player` i s'afegeix a la llista global de jugadors.
4. **Fil de Jugador**: Cada jugador es gestiona en un fil separat mitjançant la funció `handle_player`, que rep la jugada del jugador, determina el resultat del joc i envia la resposta al client.
5. **Tancament**: Quan el servidor es tanca, es tanquen tots els sockets i es neteja la llibreria Winsock.

![server](/M9_UF2_PAC5/images/server_init.png)

## Funcionament del Client

1. **Inicialització**: El client inicialitza la llibreria Winsock i crea un socket per connectar-se al servidor.
2. **Connexió al Servidor**: El client es connecta al servidor utilitzant l'adreça IP i el port especificats.
3. **Enviament de Jugada**: El client demana a l'usuari que introdueixi la seva jugada (pedra, paper, tisores, llangardaix, spock) i l'envia al servidor.
4. **Recepció del Resultat**: El client espera la resposta del servidor amb el resultat del joc i el mostra a l'usuari.
5. **Tancament**: El client tanca el socket i neteja la llibreria Winsock.