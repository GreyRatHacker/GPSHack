# GPS-Hack
Wie ich mit einem ESP32 und einem Neo 6 die Satelliten der US-Army ausspioniere. 

# NAVSTAR-Visualisierung
25-€-Ausrüstung zur Satelliten-Spionage.

Sie sind überall. Ein unsichtbares Netz, das den gesamten Planeten umspannt. Eine Multi-Milliarden-Dollar-Infrastruktur, gebaut vom US-Militär. Mit einem einfachen ESP32 und ein paar Komponenten habe ich einen Weg gefunden, sie zu sehen und ihre Bahnen live zu verfolgen.

![espSetup](https://github.com/user-attachments/assets/e975a205-6da6-4f4d-97f6-a768bb241824)


Das TFT zeigt nicht nur die Sateliten Daten und die Eigene Position. 

![9787d6fd-803f-4a4d-a802-b825e82b3c71](https://github.com/user-attachments/assets/f130e986-55e6-4f92-a6f8-b0157450522f)

Sondern auch den Speicherstatus

![59fbfd69-36a8-4806-be0a-4686d9257243](https://github.com/user-attachments/assets/74c39cc6-d57c-4267-be34-2fc2831457e2)

Dieses Repository enthält den gesamten Code und die Bauanleitung, um deine eigene Kommandozentrale zu erstellen, die GPS-Satellitendaten abfängt und sie als atemberaubende 3D-Visualisierung im Browser darstellt.


**********************************************************************************
ESP32	- 
Das Gehirn der Operation. Brutale Rechenleistung für die Entschlüsselung.

GPS-Empfänger	- 
Das unauffällige Keramik-Patch. Unsere Antenne, die die flüsterleisen Signale filtert.

SD-Karte - 
Unser Logbuch. Speichert jeden abgefangenen Datenpunkt als Beweis.

TFT-Display	- 
Unser Feld-Terminal. Zeigt live den Status der Aufzeichnung.
**********************************************************************************


"Die index.html zeigt euch dann die Satellitendaten an. Wenn ihr das nachbauen wollt, lasst uns zusammenarbeiten. Wir können die Daten gemeinsam aufzeichnen und so ein globales Überwachungsnetzwerk schaffen. Wie ihr seht, sind meine Daten bisher nur lückenhaft."

<img width="1906" height="944" alt="indexPunkthtml" src="https://github.com/user-attachments/assets/26ac3165-05fa-4897-be5f-50f3923d8c75" />


# Der Blickwinkel: Weit über 90 Grad

Ich erkläre euch, was hier passiert. Die Erde hat eine Achsenneigung von etwa 23,5°, was sie um die Z-Achse drehen lässt. Wenn wir diesen Wert einbeziehen, ergibt sich ein unglaublicher Blickwinkel von über 227° in 24 Stunden. Das ist das Ergebnis von 24 Stunden Satellitendaten, die ich gesammelt habe.

<img width="1200" height="753" alt="winkel" src="https://github.com/user-attachments/assets/e020779b-a4f3-429b-9137-ec15e636e757" />


# Das Geheimnis hinter den wilden Kurven
Ja, diese Bahnen sehen abgefahren aus! Sie zeigen, wie sich die Satelliten relativ zu einem bewegten Beobachter verhalten. Während sich mein ESP32 über die rotierende Achse bewegt, fliegen die Satelliten über ihn hinweg und erscheinen ihm aus dieser Perspektive in wilden Kurven.

Doch keine Sorge, das ist kein Fehler, sondern das Ergebnis einer präzisen Berechnung.

Mein Code nutzt die Gesetze der Geometrie, um die Bahnen korrekt darzustellen. Er berechnet den genauen Punkt, an dem eine Sichtlinie von seinem Standort die reale Satelliten-Orbithöhe schneidet. So werden die scheinbar chaotischen relativen Bewegungen in eine korrekte, absolute 3D-Position im Raum umgewandelt. Das Ergebnis sind die flüssigen, berechneten Bahnen, die ihr hier seht.



# PS: ich nutze XAMPP als Webserver

<img width="667" height="433" alt="xampp" src="https://github.com/user-attachments/assets/d2c4ca25-6cf7-42f1-b044-8a838a545dc8" />


Zum beispiel kann ich mit dieser abfrage:
**********************************************************************************
SELECT sat_id, COUNT(id) AS anzahl_sichtungen FROM satellite_readings GROUP BY sat_id ORDER BY `satellite_readings`.`sat_id` ASC;
**********************************************************************************

Die einzelnen Datenbestände für jeden Satelliten erkennen. UND, Ja. die IDs dort sind Tatsächlich von Satelliten gesendete Namen. Kreativ oder?  Ich stell mir das so vor: "General? Wie bezeichnen wir unsere Satelliten?", "IDs ist einfach, oder?", "Groovy!" 

<img width="955" height="853" alt="sql" src="https://github.com/user-attachments/assets/49364168-f3e0-416a-ba38-164441b1b57f" />


