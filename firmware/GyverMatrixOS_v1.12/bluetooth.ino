// вкладка работы с bluetooth

void sendSettingsUpdate();

byte prevY = 0;
byte prevX = 0;

boolean gameFlag;
boolean runningFlag;

#if (BT_MODE == 1)
#define PARSE_AMOUNT 4    // максимальное количество значений в массиве, который хотим получить
#define header '$'        // стартовый символ
#define divider ' '       // разделительный символ
#define ending ';'        // завершающий символ

//byte prevY = 0;
//byte prevX = 0;
//boolean runningFlag;
//boolean gameFlag;
boolean effectsFlag;
boolean recievedFlag;
byte game;
byte effect;
byte intData[PARSE_AMOUNT];     // массив численных значений после парсинга
uint32_t prevColor;
boolean msgRecieved;
byte lastMode = 0;
boolean parseStarted;

void sendSettingsUpdate() {
  
}

void bluetoothRoutine() {
  parsing();                           // принимаем данные

  if (!parseStarted && BTcontrol) {                // на время принятия данных матрицу не обновляем!

    if (runningFlag) fillString(runningText, globalColor);   // бегущая строка
    if (gameFlag) games();                      // игры
    if (effectsFlag) effects();                 // эффекты
  }
}

// блок эффектов, работают по общему таймеру
void effects() {
  if (effectTimer.isReady()) {
    switch (effect) {
      case 0: brightnessRoutine();
        break;
      case 1: colorsRoutine();
        break;
      case 2: snowRoutine();
        break;
      case 3: ballRoutine();
        break;
      case 4: rainbowRoutine();
        break;
      case 5: rainbowColorsRoutine();
        break;
      case 6: fireRoutine();
        break;
      case 7: matrixRoutine();
        break;
      case 8: ballsRoutine();
        break;
      case 9: //wavesRoutine();  // убран из этой версии, т.к. хлам
        break;
      case 10: starfallRoutine();
        break;
      case 11: sparklesRoutine();
        break;
      case 12: rainbowDiagonalRoutine();
        break;
      case 13: madnessNoise();
        break;
      case 14: cloudNoise();
        break;
      case 15: lavaNoise();
        break;
      case 16: plasmaNoise();
        break;
      case 17: rainbowNoise();
        break;
      case 18: rainbowStripeNoise();
        break;
      case 19: zebraNoise();
        break;
      case 20: forestNoise();
        break;
      case 21: oceanNoise();
        break;
    }
    FastLED.show();
  }
}

// блок игр
void games() {
  switch (game) {
    case 0:
      snakeRoutine();
      break;
    case 1:
      tetrisRoutine();
      break;
    case 2:
      mazeRoutine();
      break;
    case 3:
      runnerRoutine();
      break;
    case 4:
      flappyRoutine();
      break;
    case 5:
      arkanoidRoutine();
      break;
  }
}

byte parse_index;
String string_convert = "";
enum modes {NORMAL, COLOR, TEXT} parseMode;


// ********************* ПРИНИМАЕМ ДАННЫЕ **********************
void parsing() {
  // ****************** ОБРАБОТКА *****************
  /*
    Протокол связи, посылка начинается с режима. Режимы:
    0 - отправка цвета $0 colorHEX;
    1 - отправка координат точки $1 X Y;
    2 - заливка - $2;
    3 - очистка - $3;
    4 - яркость - $4 value;
    5 - картинка $5 colorHEX X Y;
    6 - текст $6 some text
    7 - управление текстом: $7 1; пуск, $7 0; стоп
    8 - эффект
      - $8 0 номерЭффекта;
      - $8 1 старт/стоп;
    9 - игра
    10 - кнопка вверх
    11 - кнопка вправо
    12 - кнопка вниз
    13 - кнопка влево
    14 - пауза в игре
    15 - скорость $8 скорость;
  */
  if (recievedFlag) {      // если получены данные
    recievedFlag = false;


    DBG_PRINTLN("MSG: ");
    for (byte i = 0; i < parse_index; i++) {
      DBG_PRINT(intData[i]);
      DBG_PRINT(" ");
    } 
    DBG_PRINTLN(" ");
    for (byte i = 0; i < parse_index; i++) {
      DBG_PRINT(intData[i],HEX);
      DBG_PRINT(" ");
    } 
    DBG_PRINTLN("===========");


    if (intData[0] != 16) {
      idleTimer.reset();
      idleState = false;

      if (!BTcontrol) {
        gameSpeed = globalSpeed * 4;
        gameTimer.setInterval(gameSpeed);
        BTcontrol = true;
      }
    }

    switch (intData[0]) {
      case 1:
        drawPixelXY(intData[1], intData[2], gammaCorrection(globalColor));
        FastLED.show();
        break;
      case 2:
        fillAll(gammaCorrection(globalColor));
        FastLED.show();
        break;
      case 3:
        FastLED.clear();
        FastLED.show();
        break;
      case 4:
        globalBrightness = intData[1];
        breathBrightness = globalBrightness;
        FastLED.setBrightness(globalBrightness);
        FastLED.show();
        break;
      case 5:
        drawPixelXY(intData[2], intData[3], gammaCorrection(globalColor));
        // делаем обновление матрицы каждую строчку, чтобы реже обновляться
        // и не пропускать пакеты данных (потому что отправка на большую матрицу занимает много времени)
        if (prevY != intData[3] || ( (intData[3] == 0) && (intData[2] == WIDTH - 1) ) ) {
          prevY = intData[3];
          FastLED.show();
        }
        break;
      case 6:
        loadingFlag = true;
        // строка принимается в переменную runningText
        break;
      case 7:
        if (intData[1] == 1) runningFlag = true;
        if (intData[1] == 0) runningFlag = false;
        break;
      case 8:
        if (intData[1] == 0) {
          effect = intData[2];
          gameFlag = false;
          loadingFlag = true;
          breathBrightness = globalBrightness;
          FastLED.setBrightness(globalBrightness);    // возвращаем яркость
          globalSpeed = intData[3];
          gameTimer.setInterval(globalSpeed * 4);
        }
        else if (intData[1] == 1) effectsFlag = !effectsFlag;
        break;
      case 9:
        if (lastMode != 1) loadingFlag = true;    // начать новую игру при переходе со всех режимов кроме рисования
        effectsFlag = false;
        //gameFlag = true;
        game = intData[1];
        globalSpeed = intData[2];
        gameSpeed = globalSpeed * 4;
        gameTimer.setInterval(gameSpeed);
        break;
      case 10:
        buttons = 0;
        controlFlag = true;
        break;
      case 11:
        buttons = 1;
        controlFlag = true;
        break;
      case 12:
        buttons = 2;
        controlFlag = true;
        break;
      case 13:
        buttons = 3;
        controlFlag = true;
        break;
      case 14:
        gameFlag = !gameFlag;
        break;
      case 15: globalSpeed = intData[1];
        if (gameFlag) {
          gameSpeed = globalSpeed * 4;      // для игр скорость нужно меньше!
          gameTimer.setInterval(gameSpeed);
        }
        if (effectsFlag) effectTimer.setInterval(globalSpeed);
        if (runningFlag) scrollTimer.setInterval(globalSpeed);
        break;
      case 16:
        if (intData[1] == 0) AUTOPLAY = true;
        else if (intData[1] == 1) AUTOPLAY = false;
        else if (intData[1] == 2) prevMode();
        else if (intData[1] == 3) nextMode();
        break;
      case 17: autoplayTime = ((long)intData[1] * 1000);
        autoplayTimer = millis();
        break;
    }
    lastMode = intData[0];  // запомнить предыдущий режим
  }

  // ****************** ПАРСИНГ *****************
 #if HARDWARE_BT_SERIAL
    if (Serial.available() > 0) {
#else
    if (btSerial.available() > 0) {
#endif
 // if (Serial.available() > 0) {
    char incomingByte;
    if (parseMode == TEXT) {     // если нужно принять строку
 #if HARDWARE_BT_SERIAL
        runningText = Serial.readString();
#else
        runningText = btSerial.readString();
#endif
      DBG_PRINT("runningText: ");
      DBG_PRINTLN(runningText);
      //runningText = Serial.readString();  // принимаем всю
      incomingByte = ending;              // сразу завершаем парс
      parseMode = NORMAL;
    } else {
#if HARDWARE_BT_SERIAL
        incomingByte = Serial.read(); 
#else
        incomingByte = btSerial.read(); 
#endif
        DBG_PRINT("b: ");
        DBG_PRINT(incomingByte,HEX);
        DBG_PRINT(" - ");
        DBG_PRINTLN(incomingByte);
    //  incomingByte = Serial.read();        // обязательно ЧИТАЕМ входящий символ
    }
    if (parseStarted) {                         // если приняли начальный символ (парсинг разрешён)
      if (incomingByte != divider && incomingByte != ending) {   // если это не пробел И не конец
        string_convert += incomingByte;       // складываем в строку
      } else {                                // если это пробел или ; конец пакета
        if (parse_index == 0) {
          byte thisMode = string_convert.toInt();
          if (thisMode == 0 || thisMode == 5) parseMode = COLOR;    // передача цвета (в отдельную переменную)
          else if (thisMode == 6) parseMode = TEXT;
          else parseMode = NORMAL;
          //if (thisMode != 7 || thisMode != 0) runningFlag = false;
        }

        if (parse_index == 1) {       // для второго (с нуля) символа в посылке
          if (parseMode == NORMAL) intData[parse_index] = string_convert.toInt();             // преобразуем строку в int и кладём в массив}
          //if (parseMode == COLOR) globalColor = strtol(&string_convert[0], NULL, 16);     // преобразуем строку HEX в цифру
          if (parseMode == COLOR) globalColor = (uint32_t)HEXtoInt(string_convert);     // преобразуем строку HEX в цифру
        } else {
          intData[parse_index] = string_convert.toInt();  // преобразуем строку в int и кладём в массив
        }
        string_convert = "";                  // очищаем строку
        parse_index++;                              // переходим к парсингу следующего элемента массива
      }
    }
    if (incomingByte == header) {             // если это $
      parseStarted = true;                      // поднимаем флаг, что можно парсить
      parse_index = 0;                              // сбрасываем индекс
      string_convert = "";                    // очищаем строку
    }
    if (incomingByte == ending) {             // если таки приняли ; - конец парсинга
      parseMode == NORMAL;
      parseStarted = false;                     // сброс
      recievedFlag = true;                    // флаг на принятие
    }
  }
}

// hex string to uint32_t
uint32_t HEXtoInt(String hexValue) {
  byte tens, ones, number1, number2, number3, number4;
  tens = (hexValue[0] < '9') ? hexValue[0] - '0' : hexValue[0] - '7';
  ones = (hexValue[1] < '9') ? hexValue[1] - '0' : hexValue[1] - '7';
  number1 = (16 * tens) + ones;

  tens = (hexValue[2] < '9') ? hexValue[2] - '0' : hexValue[2] - '7';
  ones = (hexValue[3] < '9') ? hexValue[3] - '0' : hexValue[3] - '7';
  number2 = (16 * tens) + ones;

  tens = (hexValue[4] < '9') ? hexValue[4] - '0' : hexValue[4] - '7';
  ones = (hexValue[5] < '9') ? hexValue[5] - '0' : hexValue[5] - '7';
  number3 = (16 * tens) + ones;

  return ((uint32_t)number1 << 16 | (uint32_t)number2 << 8 | number3 << 0);
}



#elif (BT_MODE == 2)

#define HEADER_SYMBOL '$'

boolean msgRecieved;
boolean getStarted;

uint32_t parseTimer;
uint16_t parseWaitPeriod = 2*1000; // timeout 2 sec

uint8_t bleMsg[255];
byte byte_index = 0;
byte expected_bytes = 0;

void sendCommand(uint8_t cmd[], uint8_t len) {
#if HARDWARE_BT_SERIAL
    Serial.write(cmd,len);
#else
    btSerial.write(cmd,len);
#endif
}

void sendSettingsUpdate() {
    sendSettings(true);
}

void sendSettings() {
    sendSettings(false);
}

void sendSettings(bool isNotification) {
    uint8_t cmd[7];
    cmd[0] = isNotification ? 0xB4 : 0xB5;
    cmd[1] = 0x05;
    cmd[2] = AUTOPLAY ? 0x01 : 0x00;
    cmd[3] = thisMode;
    cmd[4] = globalBrightness;
    cmd[5] = globalSpeed;
    cmd[6] = (uint8_t)(autoplayTime/1000/60);
    sendCommand(cmd,cmd[1]+2);
}

void successResponse() {
    uint8_t cmd[3]; 
    cmd[0] = 0xB0;
    cmd[1] = 0x01;
    cmd[2] = 0x01;
    sendCommand(cmd,cmd[1]+2);
}

void switchToGame(uint8_t game) {
  switch (game) {
    case 0:
      snakeRoutine();
      break;
    case 1:
      tetrisRoutine();
      break;
    case 2:
      mazeRoutine();
      break;
    case 3:
      runnerRoutine();
      break;
    case 4:
      flappyRoutine();
      break;
    case 5:
      arkanoidRoutine();
      break;
  }
}

void applyEffect(uint8_t effect) {
      FastLED.clear();
      FastLED.show();
      thisMode = effect;
      autoplayTimer = millis();
}

void bluetoothRoutine() {
    incomingMsgHandler();
    if (msgRecieved) {
        handleNewMsg();
    }
}

void handleNewMsg() {
    msgRecieved = false;

    DBG_PRINTLN("msgRecieved");

    bool needSendSuccessResponse = false;
    bool needSendSettingsResponse = false;

    switch (bleMsg[0]) {
        case 0:
            needSendSettingsResponse = true;
            break;
        case 1: 
            drawPixelXY(bleMsg[1], bleMsg[2], gammaCorrection(globalColor));
            FastLED.show();
            needSendSuccessResponse = true;
            break;
        case 2:
            fillAll(gammaCorrection(globalColor));
            FastLED.show();
            needSendSuccessResponse = true;
            break;
        case 3:
            FastLED.clear();
            FastLED.show();
            needSendSuccessResponse = true;
            break;
        case 4:
            globalBrightness = bleMsg[1];
            breathBrightness = globalBrightness;
            FastLED.setBrightness(globalBrightness);
            FastLED.show();
            needSendSuccessResponse = true;
            break;
        case 5:
            drawPixelXY(bleMsg[2], bleMsg[3], gammaCorrection(globalColor));
            // делаем обновление матрицы каждую строчку, чтобы реже обновляться
            // и не пропускать пакеты данных (потому что отправка на большую матрицу занимает много времени)
            if (prevY != bleMsg[3] || ( (bleMsg[3] == 0) && (bleMsg[2] == WIDTH - 1) ) ) {
                prevY = bleMsg[3];
                FastLED.show();
            }
            needSendSuccessResponse = true;
            break;
        case 6:
            loadingFlag = true;
            needSendSuccessResponse = true;
            break;
        case 7:
            runningFlag = bleMsg[1] == 1;
            needSendSuccessResponse = true;
            break;
        case 8:
            if (bleMsg[1] == 0) {
                gameFlag = false;
                loadingFlag = true;
                breathBrightness = globalBrightness;
                FastLED.setBrightness(globalBrightness);    // возвращаем яркость
                applyEffect(bleMsg[2]);
            } 
            needSendSuccessResponse = true;
            break;
        case 9:
            globalSpeed = bleMsg[2];
            gameSpeed = globalSpeed * 4;
            gameTimer.setInterval(gameSpeed);
            switchToGame(bleMsg[1]);
            needSendSuccessResponse = true;
            break;
        case 10:
            buttons = bleMsg[1];
            controlFlag = true;
            needSendSuccessResponse = true;
            break;
        case 11:
            break;
        case 12:
            break;
        case 13:
            break;
        case 14:
            gameFlag = !gameFlag;
            needSendSuccessResponse = true;
            break;
        case 15:
            globalSpeed = bleMsg[1];
            effectTimer.setInterval((int)globalSpeed+30);
            needSendSuccessResponse = true;
            break;
        case 16:
            if (bleMsg[1] == 0) {
                AUTOPLAY = true;
                needSendSettingsResponse = true;
            } else if (bleMsg[1] == 1) {
                AUTOPLAY = false;
                needSendSettingsResponse = true;
            } else if (bleMsg[1] == 2) {
                prevMode();
                // a message with the settings will be send after switch to next mode
            } else if (bleMsg[1] == 3) {
                nextMode();
                // a message with the settings will be send after switch to next mode
            }
            
            break;
        case 17:
            autoplayTime = ((long)bleMsg[1] * 60 * 1000);
            autoplayTimer = millis();
            needSendSuccessResponse = true;
            break;                                   
    }   

    if (needSendSuccessResponse) {
        successResponse();
    }

    if (needSendSettingsResponse) {
        sendSettings();
    }
}

void incomingMsgHandler() {

    // the check for make sure we are not freazing in the case wrong message
    if (getStarted) {
        if ((millis() - parseTimer >= parseWaitPeriod)) {
            parseTimer = millis();
            getStarted = false;
            expected_bytes = 0;
            byte_index = 0;
            DBG_PRINTLN("Reset parse msg by timeout");
        }
    }

#if HARDWARE_BT_SERIAL
    if (Serial.available() > 0) {
        uint8_t incomingByte = Serial.read();      // обязательно ЧИТАЕМ входящий символ
#else
    if (btSerial.available() > 0) {
        uint8_t incomingByte = btSerial.read();      // обязательно ЧИТАЕМ входящий символ
#endif
        parseTimer = millis();

        DBG_PRINT("b: ");
        DBG_PRINT(incomingByte,HEX);
        DBG_PRINT(" - ");
        DBG_PRINTLN(incomingByte);
    
        if (getStarted) {
            if (expected_bytes == 0) {
                expected_bytes = incomingByte;
                DBG_PRINT("expected_bytes: ");
                DBG_PRINTLN(expected_bytes);
            } else {
                bleMsg[byte_index] = incomingByte;
                byte_index++;
            }

//            DBG_PRINT("byte_index: ");
//            DBG_PRINTLN(byte_index);

            if (expected_bytes == byte_index) {
                getStarted = false;
                msgRecieved = true;
            }
        } else {
            if (incomingByte == HEADER_SYMBOL) {
                getStarted = true;
                expected_bytes = 0;
                byte_index = 0;
            }
        }
    }
}

#elif (BT_MODE == 0)
void bluetoothRoutine() {
  return;
}
void sendSettingsUpdate() {
  
}
#endif
