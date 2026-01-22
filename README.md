# Tutka-oviavausjärjestelmä
Sulautetut järjestelmät – Harjoitustyö  
**Tiimi:** ENER-Gy  
**Tekijät:**  
- **Eino Lausmaa** ([@eino](https://github.com/LausmaaEino)) – Piirin /  3D:n rakentaja
- **Eemeli Ranta** ([@eemeli](https://github.com/Elmadus)) – Projektivastaava
- **Nhan Phan** ([@nio](https://github.com/niophan)) – Tekninen vetäjä
---

## 📌 Projektin kuvaus
Tämä projekti toteuttaa ovenavausjärjestelmän, joka hyödyntää tutkaa ja sulautettuja järjestelmiä. Järjestelmä perustuu **Arduino Nano** -mikrokontrollereihin ja sisältää useita komponentteja, kuten ultraäänianturin, servomoottorit, LCD-näytön, näppäimistön ja äänimerkin.  
Tavoitteena oli rakentaa monipuolinen ja innovatiivinen ratkaisu, joka hyödyntää kurssilla opittuja tekniikoita: keskeytyksiä, rekisteriohjausta, tiedonsiirtoa ja laitteiden integrointia.

---

## 🎯 Tavoitteet
- Innovatiivinen ratkaisu oven avaamiseen tutkan avulla  
- Käyttää vähintään **4 sisäistä IO-lohkoa**, joista osa rekisteriohjauksella  
- Toteuttaa **2 keskeytyspalvelua**  
- Sisällyttää **tiedonsiirtoyhteys** (Nano ↔ Nano, SoftwareSerial)  
- Panostaa rakenteeseen (mm. 3D-tulostetut osat)  

---

## 🔧 Komponentit
- 2 × Arduino Nano  
- Ultraäänianturi (URM09)  
- 2 × Servomoottori (tutkan ja oven ohjaus)  
- LCD-näyttö (I2C-väylä)  
- 4×4 näppäimistö  
- Kaiutin/hälytin  
- LED-diodit (punainen, vihreä)  
- Vastukset ja kondensaattorit  

---

## 📐 Rakenne ja toiminta
- **Nano #1**: ohjaa tutkaa, servoja, näyttöä ja oven mekanismia  
- **Nano #2**: käsittelee näppäimistön syötteet ja välittää ne Nano #1:lle SoftwareSerial-yhteydellä  
- Ultraäänianturi skannaa ympäristöä 180° kulmassa servon avulla  
- Kun objekti havaitaan <10 cm etäisyydellä, järjestelmä siirtyy **salasanatilaan**  
- Oikea koodi avaa oven ja tarjoaa mahdollisuuden vaihtaa salasana EEPROM-muistiin  
- Väärät yritykset → lukitustila (1 min), aikakatkaisu (7 s)  

---

## 💻 Ohjelmisto
- **Käytetyt kirjastot:**  
  - Servo.h  
  - DFRobot_RGBLCD1602.h  
  - SoftwareSerial.h  
  - EEPROM.h  
  - avr/wdt.h (vahtikoira)  
- **Keskeiset ominaisuudet:**  
  - Servo-ohjaus PWM:llä  
  - EEPROM-salasana  
  - Watchdog-reset  
  - Nano ↔ Nano tiedonsiirto SoftwareSerialilla  
  - LCD-näytön hallinta I2C:llä  
  - Äänimerkit ja melodiat  

---

## 📷 Kuvia
![Piirikaavio](https://github.com/niophan/IoT-automation-door/blob/main/Project-Photos/20251209_114740204_iOS.jpeg)
![Kokonaisuus ja komponentit](https://github.com/niophan/IoT-automation-door/blob/main/Project-Photos/20251209_114304947_iOS.jpeg)
![Näppäimistö ja näyttö](https://github.com/niophan/IoT-automation-door/blob/main/Project-Photos/20251209_114353207_iOS.jpeg)
![Ovimekanismi](https://github.com/niophan/IoT-automation-door/blob/main/Project-Photos/20251209_114754436_iOS.jpeg)
![Tutka](https://github.com/niophan/IoT-automation-door/blob/main/Project-Photos/20251209_114749679_iOS.jpeg)

---

## ▶ Testaus ja toiminta
- Näyttö näyttää tutkan kulman ja etäisyyden reaaliajassa  
- Objekti havaittu → punainen tausta, “Enter Code”  
- **Oikea koodi:** vihreä tausta, hyväksymismelodia, ovi aukeaa  
- **Väärä koodi:** virheilmoitus, äänimerkki, lukitus 3 virheestä  
- **Aikakatkaisu:** 7 s ilman syötettä → palaa tutkailutilaan  

---

## 📹 Demo
[Katso video](https://tuni-my.sharepoint.com/:v:/r/personal/nio_phan_tuni_fi/Documents/Sulautetun%20järjestelmän%20ohjelmointi%20ja%20mikrokontrollerit/harjoitus1/20251209_131230000_iOS.MP4?csf=1&web=1&nav=eyJyZWZlcnJhbEluZm8iOnsicmVmZXJyYWxBcHAiOiJPbmVEcml2ZUZvckJ1c2luZXNzIiwicmVmZXJyYWxBcHBQbGF0Zm9ybSI6IldlYiIsInJlZmVycmFsTW9kZSI6InZpZXciLCJyZWZlcnJhbFZpZXciOiJNeUZpbGVzTGlua0NvcHkifX0&e=OJn8v9)

---

## 📚 Pohdinta
Projekti tarjosi arvokasta oppia sulautettujen järjestelmien suunnittelusta ja komponenttien integroinnista. Haasteita syntyi mm. 3D-tulostuksessa ja aikataulussa, mutta lopputulos oli toimiva ja monipuolinen prototyyppi.
