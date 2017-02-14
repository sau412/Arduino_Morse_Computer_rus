#include <EEPROM.h>
#include <avr/pgmspace.h>

// Порты ввода-вывода
#define BUTTON_PIN 11
#define BUZZER_PIN 3
#define LED_PIN LED_BUILTIN

// Вывод заметок на последовательный порт при подключении
#define SERIAL_DUMP
#define SERIAL_DUMP_GREETING F("Zametki:")
#define SERIAL_DUMP_ENDING F("")
// Режим отладки
#define SERIAL_DEBUG

// Типы символов для н назад и памяти
#define SYMBOLS_ALPHA 1
#define SYMBOLS_NUM 2
#define SYMBOLS_SPECIAL 3
#define SYMBOLS_ALPHA_NUM 4
#define SYMBOLS_ALL 5
const char symbols_alpha_list[] PROGMEM = "abvgdejziyklmnoprstufhc\x04w\x05\x06\x07\x08q\x09";
const char symbols_num_list[] PROGMEM = "0123456789";
const char symbols_special_list[] PROGMEM = ".,:;'\"-/?!@";

// Число точек в тире
#define DOTS_IN_DASH 3

// Значения по умолчанию (после сброса)
#define SETTINGS_VERSION 2
#define DEFAULT_DOT_LEN 200
#define DEFAULT_ECHO_DOT_LEN 100
#define DEFAULT_EXTRA_PAUSE 0
#define DEFAULT_LED_ENABLED 1
#define DEFAULT_BUZZER_ENABLED 1
#define DEFAULT_BULLS_AND_COWS_LEN 3
#define DEFAULT_FOXES_LEN 5
#define DEFAULT_FOXES_COUNT 5
#define DEFAULT_N_BACK_LEN 2
#define DEFAULT_N_BACK_TYPE SYMBOLS_ALPHA
#define DEFAULT_MEMORY_LEN 3
#define DEFAULT_MEMORY_TYPE SYMBOLS_ALPHA

// EEPROM
#define EEPROM_NOTES_OFFSET 0
#define EEPROM_NOTES_LEN 400
#define EEPROM_SETTINGS_OFFSET 400

// Длина буферов
// Буфер, содержащий код выводимого символа (макисмальная длина символа)
#define SYMBOL_BUFFER_MAX 10
// Буфер, содержащий код вводимого символа
#define INPUT_BUFFER_MAX 10
// Буфер, содержащий число при выводе числа
#define NUMBER_BUFFER_MAX 15
// Буфер заметки
#define NOTE_BUFFER_MAX 100
// Макисмальная длина числа в памяти
#define MEMORY_LEN_MAX 10
// Макисмальная длина очереди в н назад
#define N_BACK_LEN_MAX 10
// Максимальная длина числа в быках и коровах
#define BULLS_AND_COWS_LEN_MAX 10
// Буфер для быков и коров
#define BULLS_AND_COWS_BUFF 10
// Охота на лис максимальный размер поля
#define FOXES_FIELD_SIZE_MAX 10
// Охота на лис максимальное число лис
#define FOXES_COUNT_MAX 10
// Буфер для охоты на лис
#define FOXES_BUFF 10
// Буфер для циклического повтора фразы
#define CYCLE_MAX_LEN 100

// Символы меню
// Меню
#define MENU_SYMBOL 'm'
// Добавить заметку
#define NOTE_SYMBOL 'z'
// Чтение заметок
#define READ_SYMBOL 4
// Справка
#define HELP_SYMBOL 's'
// Аптайм
#define UPTIME_SYMBOL 'a'
// Эхо
#define ECHO_SYMBOL 7
// Калькулятор
#define CALC_SYMBOL 'k'
// Таймер
#define TIMER_SYMBOL 't'
// Охота на лис
#define FOXES_SYMBOL 'o'
// Быки и коровы
#define BULLS_SYMBOL 'b'
// Н назад
#define NBACK_SYMBOL 'n'
// Память
#define MEMORY_SYMBOL 'p'
// Устный счёт
#define COUNT_SYMBOL 'u'
// Повтор фразы
#define CYSLE_SYMBOL 'c'
// Фонарик
#define LIGHT_SYMBOL 'f'

// Специальные символы
// Конец связи (для выхода в меню)
#define END_SYMBOL 13
// Ошибка (для выхода в меню без сохранения)
#define ERROR_SYMBOL 14
// Символ победы
#define WIN_SYMBOL 15
// Знак раздела
#define BREAK_SYMBOL 15
// Пробел
#define SPACE_SYMBOL ' '

// Первый символ в строке - кодируемый символ, далее идёт код кодом морзе, в конце кода перевод строки
// Пробел кодируется как два пробела потому что после символа идёт пауза в три точки
const char morse_codes[] PROGMEM = "\
   \0\
a.-\0\
b-...\0\
v.--\0\
g--.\0\
d-..\0\
e.\0\
j...-\0\
z--..\0\
i..\0\
y.---\0\
k-.-\0\
l.-..\0\
m--\0\
n-.\0\
o---\0\
p.--.\0\
r.-.\0\
s...\0\
t-\0\
u..-\0\
f..-.\0\
h....\0\
c-.-.\0\
\x04---.\0\
w----\0\
\x05-..-\0\
\x06--.-\0\
\x07-.--\0\
\x08..-..\0\
q..--\0\
\x09.-.-\0\
\x30-----\0\
\x31.----\0\
\x32..---\0\
\x33...--\0\
\x34....-\0\
\x35.....\0\
\x36-....\0\
\x37--...\0\
\x38---..\0\
\x39----.\0\
.......\0\
,.-.-.-\0\
:---...\0\
;-.-.-.\0\
'.----.\0\
\".-..-.\0\
--....-\0\
/-..-.\0\
?..--..\0\
!--..--\0\
@.--.-.\0\
\x0d..-.-\0\
\x0e........\0\
\x0f-...-\0\
";


// ===================================================
// 
// Настройки
// 
// ===================================================

// ===============================================
// Структура настроек
// ===============================================
struct settings {
  // Версия настроек
  int ver;
  // Настройки ввода-вывода
  int dot_length;          // Длительность точки
  int pause_extra_length;  // Удлиннение паузы между буквами
  int echo_length;         // Длительность эха
  char is_buzzer_enabled;  // Включён ли звук
  char is_led_enabled;     // Включён ли свет
  // Регистры калькулятора
  long calc_register_a;    // Регистр А
  long calc_register_b;    // Регистр Б
  long calc_register_m;    // Регистр М
  // Настройки быков и коров
  char bulls_and_cows_len; // Длина загаданного числа
  // Настройки охоты на лис
  char foxes_field_size;   // Размер поля
  char foxes_count;        // Число лис
  // Настройки н назад
  char n_back_len;         // Длина очереди
  char n_back_type;        // Тип символов: только цифры, только буквы, только знаки, цифры+буквы, цифры+буквы+знаки
  // Настройки запоминания
  char memory_len;         // Количетсво запоминаемых элементов
  char memory_type;        // Тип символов: только цифры, только буквы, только знаки, цифры+буквы, цифры+буквы+знаки
} settings;

// ===============================================
// Сохранение настроек
// ===============================================
void settings_save() {
  // Записываем структуру в память
  EEPROM.put(EEPROM_SETTINGS_OFFSET,settings);
}

// ===============================================
// Сохранение настроек
// ===============================================
void settings_load() {
  // Читаем структуру из памяти
  EEPROM.get(EEPROM_SETTINGS_OFFSET,settings);
  // При несоответствии версии загружаем стандартные настройки
  if(settings.ver!=SETTINGS_VERSION) {
    settings_reset();
  }
}

// ===============================================
// Сброс настроек
// ===============================================
void settings_reset() {
  // Настройки ввода-вывода
  settings.dot_length=DEFAULT_DOT_LEN;
  settings.pause_extra_length=DEFAULT_EXTRA_PAUSE;
  settings.echo_length=DEFAULT_ECHO_DOT_LEN;
  settings.is_buzzer_enabled=DEFAULT_BUZZER_ENABLED;
  settings.is_led_enabled=DEFAULT_LED_ENABLED;
  // Калькулятор
  settings.calc_register_a=0;
  settings.calc_register_b=0;
  settings.calc_register_m=0;
  // Быки и коровы
  settings.bulls_and_cows_len=DEFAULT_BULLS_AND_COWS_LEN;
  // Охота на лис
  settings.foxes_field_size=DEFAULT_FOXES_LEN;
  settings.foxes_count=DEFAULT_FOXES_COUNT;
  // Н назад
  settings.n_back_len=DEFAULT_N_BACK_LEN;
  settings.n_back_type=DEFAULT_N_BACK_TYPE;
  // Запоминание
  settings.memory_len=DEFAULT_MEMORY_LEN;
  settings.memory_type=DEFAULT_MEMORY_TYPE;
}


// ===================================================
// 
// Ввод и вывод
// 
// ===================================================

// ===============================================
// Мигнуть диодом и издать звук (как настроено)
// ===============================================
void output_single_blink(long high_len,long low_len) {
  long start;

  // Если нажата кнопка - прекращаем вывод
  if(digitalRead(BUTTON_PIN)==LOW) return;
  
  // Если в настройках разрешено, включаем диод
  if(settings.is_led_enabled==1) {
    digitalWrite(LED_PIN, HIGH);
  }
  
  // Промежуток наличия сигнала
  start=millis();
  while(millis()-start<=high_len) {
    if(digitalRead(BUTTON_PIN)==LOW) return;
    // Если звук включен - издаём его
    if(settings.is_buzzer_enabled==1) {
      digitalWrite(BUZZER_PIN, HIGH);
      digitalWrite(BUZZER_PIN, LOW);
    }
    delay(1);
  }

  // Если в настройках разрешено, выключаем диод
  if(settings.is_led_enabled==1) {
    digitalWrite(LED_PIN, LOW);
  }

  // Промежуток отсутсвия сигнала
  start=millis();
  while(millis()-start<=low_len) {
    if(digitalRead(BUTTON_PIN)==LOW) return;
    delay(1);
  }
}

// ===============================================
// Вывести символ с указанной скоростью
// ===============================================
int output_char(char symbol,int char_speed) {
  char buff[SYMBOL_BUFFER_MAX];
  int pos;
  char testing_symbol;

  // Если нажата кнопка - прекращаем вывод
  if(digitalRead(BUTTON_PIN)==LOW) return 0;
  
  // Инициализируем массив
  memset(buff,0,SYMBOL_BUFFER_MAX);
  
  pos=0;
  do {
    // Читам символ из таблицы кодировки
    testing_symbol=pgm_read_byte_near(morse_codes+pos);
    
    // Если символ совпал - выводим его
    // Копируем только код, без символа
    if(testing_symbol==symbol) {
      strcpy_P(buff,morse_codes+pos+sizeof(char));
      break;
    }
    
    // Сдвигаем указатель на длину строки плюс нулевой байт
    pos+=strlen_P(morse_codes+pos)+sizeof(char);
    // Пока код символа не будет равен нулю, это значит таблица символов закончилась, и символов не найдено
  } while(testing_symbol!='\0');

  // Если код символа не указан - возвращаем 0
  if(strlen(buff)==0 || testing_symbol=='\0') return 0;

#ifdef SERIAL_DEBUG
  Serial.print("Output ");
  Serial.print(buff);
  if(isAlphaNumeric(symbol)) {
    Serial.print(" symbol '");
    Serial.print((char)symbol);
    Serial.println("'");
  } else {
    Serial.print(" symbol code ");
    Serial.println((int)symbol);
  }
#endif

  // Мигаем код, заполненный выше в функции
  for(pos=0;pos!=strlen(buff);pos++) {
    switch(buff[pos]) {
      // Символ точки означает точку
      case '.': output_single_blink(char_speed,char_speed); break;
      // Символ тире означает тире
      case '-': output_single_blink(char_speed*DOTS_IN_DASH,char_speed); break;
      // Пробел - пауза в одну точку
      case ' ': delay(char_speed); break;
      // Любой другой символ (конец строки, например) - выдерживаем паузу между символами и выходим
      default:
        break;
    }
  }
  delay(char_speed*DOTS_IN_DASH);
  return 1;
}

// ===============================================
// Вывести символ как обычно
// ===============================================
int output_char_normal(char symbol) {
  output_char(symbol,settings.dot_length);
}

// ===============================================
// Вывести символ как эхо (по умолчанию это быстрее)
// ===============================================
int output_char_echo(char symbol) {
  output_char(symbol,settings.echo_length);
}

// ===============================================
// Вывести строку
// ===============================================
void output_string(char str[]) {
  int length=strlen(str);
  int i;

#ifdef SERIAL_DEBUG
  Serial.print("Output string: ");
  Serial.println(str);
#endif

  for(i=0;i!=length;i++) {
    // Если нажата кнопка - прекращаем вывод
    if(digitalRead(BUTTON_PIN)==LOW) return;
    output_char_normal(str[i]);
  }
  // Пауза в конце строки
  output_char_normal(' ');
}

// ===============================================
// Вывести число (часто бывает нужно)
// ===============================================
void output_number(long number) {
  char buff[NUMBER_BUFFER_MAX];
  sprintf(buff,"%ld",number);
  return output_string(buff);
}

// ===============================================
// Случайный символ
// ===============================================
int output_random(char type) {
  int symbol,found;
  found=0;
  do {
    symbol=random(256);
    if(symbol==END_SYMBOL) continue;
    if(symbol==ERROR_SYMBOL) continue;
    if(symbol==SPACE_SYMBOL) continue;
    switch(type) {
      case SYMBOLS_ALPHA: if(strchr_P(symbols_alpha_list,symbol)) found=1; break;
      case SYMBOLS_NUM: if(strchr_P(symbols_num_list,symbol)) found=1; break;
      case SYMBOLS_SPECIAL: if(strchr_P(symbols_special_list,symbol)) found=1; break;
      case SYMBOLS_ALPHA_NUM: if(strchr_P(symbols_alpha_list,symbol) || strchr_P(symbols_num_list,symbol)) found=1; break;
      case SYMBOLS_ALL: if(strchr_P(symbols_alpha_list,symbol) || strchr_P(symbols_num_list,symbol) || strchr_P(symbols_special_list,symbol)) found=1; break;
    }  
  } while(found==0);
  
  output_char_normal(symbol);
  return symbol;
}

// ===============================================
// Сыграть победу
// ===============================================
void output_win() {
  output_char_echo(WIN_SYMBOL);
}

// ===============================================
// Сыграть ошибку
// ===============================================
void output_error() {
  output_char_echo(ERROR_SYMBOL);
}

// ===============================================
// Сыграть тревогу (таймер)
// ===============================================
void output_alarm() {
  output_char_echo(ERROR_SYMBOL);
}

// ===============================================
// Прочитать один символ
// ===============================================
int input_char() {
  int is_pause;
  int state;
  long start_timestamp;
  long interval;
  char buff_input[INPUT_BUFFER_MAX];
  int buff_index;
  int pos;
  char symbol;
  
  // Если кнопка уже нажата, ждём пока её отпустят
  while(digitalRead(BUTTON_PIN)!=HIGH);
  // Защита от дребезга
  delay(100);
  // Ждём нажатий
  while(digitalRead(BUTTON_PIN)!=LOW);
  
  // Инициализируем переменные
  is_pause=0;
  start_timestamp=millis();
  state=HIGH;
  memset(buff_input,0,INPUT_BUFFER_MAX);
  buff_index=0;
  
  // Дальше начинаем чтение
  while(is_pause!=1) {
    if(digitalRead(BUTTON_PIN)==HIGH && state==LOW) {
      interval=millis()-start_timestamp;
      if(interval>70) {
        is_pause=0;
        if(interval>300) { buff_input[buff_index]='-'; }
        else { buff_input[buff_index]='.'; }
        buff_index++;
        if(buff_index==INPUT_BUFFER_MAX) return 0; // Ошибка
      }
      start_timestamp=millis();
      state=HIGH;
    }
    if(digitalRead(BUTTON_PIN)==LOW && state==HIGH) {
      start_timestamp=millis();
      state=LOW;
      is_pause=0;
    }
    if(is_pause==0 && state==HIGH && (millis()-start_timestamp)>1000) {
      //Serial.println(" done");
      is_pause=1;
    }
  }

#ifdef SERIAL_DEBUG
  Serial.print("Input ");
  Serial.print(buff_input);
#endif

  pos=0;
  do {
    symbol=pgm_read_byte_near(morse_codes+pos);
    if(strcmp_P(buff_input,morse_codes+pos+sizeof(char))==0) {
      break;
    }
    pos+=strlen_P(morse_codes+pos)+sizeof(char);
  } while(symbol!='\0');

#ifdef SERIAL_DEBUG
  //Serial.print(" symbol ");
  if(isAlphaNumeric(symbol)) {
    Serial.print(" symbol '");
    Serial.print((char)symbol);
    Serial.println("'");
  } else {
    Serial.print(" symbol code ");
    Serial.println((int)symbol);
  }
#endif

  return symbol;
}

// ===============================================
// Преобразовать символ в число
// ===============================================
char symbol_to_number(char symbol) {
  if(isDigit(symbol)) return symbol-'0';
  else return -1;
}

// ===================================================
// 
// Заметки
// 
// ===================================================

// ===============================================
// Новая заметка
// ===============================================
void note_add() {
  int symbol;
  char message[NOTE_BUFFER_MAX];
  int len,i,j;

  output_char_echo(NOTE_SYMBOL);
  
  len=0;
  memset(message,0,NOTE_BUFFER_MAX);
  
  do {
    symbol=input_char();
    switch(symbol) {
        case ERROR_SYMBOL:
          if(len>0) len--;
          message[len]=0;
          break;
        case END_SYMBOL: break;
        default:
          output_char_echo(symbol);
          message[len]=symbol;
          if((len+sizeof(char))<NOTE_BUFFER_MAX) len++;
          else output_char_echo(ERROR_SYMBOL);
          break;
      }
  } while(symbol!=END_SYMBOL);
  
  // Сдвигаем существующие заметки на длину новой заметки+1
  for(i=EEPROM_NOTES_LEN-1-len-1;i>=0;i--) {
    symbol=EEPROM.read(EEPROM_NOTES_OFFSET+i);
    EEPROM.update(EEPROM_NOTES_OFFSET+i+len+1,symbol);
    /*Serial.print("Move from ");
    Serial.print(i);
    Serial.print(" to ");
    Serial.print(i+len+1);
    Serial.print(" byte ");
    Serial.println(symbol);*/
  }
  
  // Записываем новую
  for(i=0;i!=len;i++) {
    EEPROM.update(EEPROM_NOTES_OFFSET+i,message[i]);
  }
  // Записываем нулевой байт в конец
  EEPROM.update(EEPROM_NOTES_OFFSET+len,0);
}

// ===============================================
// Чтение заметок
// ===============================================
void note_read()
{
  int i;
  int symbol;
  int note_index;
  
  output_char_echo(READ_SYMBOL);
  /*for(i=0;i!=NOTES_MEMORY;i++) {
    symbol=EEPROM.read(i);
    if(symbol==0) Serial.println("<EOL>");
    else Serial.print((char)symbol);
  }*/
  note_index=0;
  do {
    note_read_by_number(note_index);
    symbol=input_char();
    switch(symbol) {
      case ERROR_SYMBOL: return; break;
      case END_SYMBOL: return; break;
      // Следующая
      case 's':
        note_index++;
        break;
      // Предыдущая
      case 'p':
        if(note_index>0) note_index--;
        break;
      // Включить циклический повтор
      case 'c':
        while(digitalRead(BUTTON_PIN)!=LOW) {
          note_read_by_number(note_index);
        }
        break;
      
      // Быстрый переход по первым 9 заметкам
      case '1':
      case '2':
      case '3':
      case '4':
      case '5':
      case '6':
      case '7':
      case '8':
      case '9':
        note_index=symbol_to_number(symbol)-1;
        break;
    }
  } while(symbol!=END_SYMBOL);
}

// ===============================================
// Прочитать заметку под номером note_index
// ===============================================
void note_read_by_number(int note_index)
{
  int symbol,i,count;
  // Пропускаем первые note_index заметок
  if(note_index>0) {
    count=0;
    for(i=0;i!=EEPROM_NOTES_LEN;i++) {
      symbol=EEPROM.read(EEPROM_NOTES_OFFSET+i);
      //Serial.print((char)symbol);
      if(symbol==0) count++;
      if(count==note_index) {
        break;
      }
    }
    // Сдвигаемся ещё на один символ
    i++;
  }
  for(;i<EEPROM_NOTES_LEN;i++) {
    symbol=EEPROM.read(EEPROM_NOTES_OFFSET+i);
    if(symbol==0) {
      output_char_normal(END_SYMBOL);
      return;
    }
    output_char_normal(symbol);
    }
}

// ===============================================
// Стереть все заметки
// ===============================================
void erase_all_notes() {
  int i;
  for(i=0;i!=EEPROM_NOTES_LEN;i++)
    EEPROM.update(EEPROM_NOTES_OFFSET+i,0);
}

// ===============================================
// Повтор фразы
// ===============================================
void cycle() {
  int cycle_buffer[CYCLE_MAX_LEN];
  int symbol,index;

  memset(cycle_buffer,0,CYCLE_MAX_LEN);
  
  // Сообщаем где мы
  output_char_echo(CYCLE_SYMBOL);

  index=0;
  do {
    symbol=input_char();
    switch(symbol) {
      case END_SYMBOL:
        break;
      case ERROR_SYMBOL:
        index--;
        cycle_buffer[index]=0;
      default:
        cycle_buffer[index]=symbol;
        if((index+1)<CYCLE_MAX_LEN) index++;
    }
  } while(symbol!=END_SYMBOL);

  while(digitalRead(BUTTON_PIN)!=LOW) {
    output_string(cycle_buffer);
  }
}

// ===================================================
// 
// Приложения
// 
// ===================================================

// ===============================================
// Калькулятор
// ===============================================
void calculator()
{
  int symbol;
  char error_flag;
  long a,b;
  char operation;
  
  output_char_echo(CALC_SYMBOL);

  b=0;
  a=0;
  operation=0;
  error_flag=0;
  do {
      symbol=input_char();
      switch(symbol) {
          // Цифры
          case '0':
          case '1':
          case '2':
          case '3':
          case '4':
          case '5':
          case '6':
          case '7':
          case '8':
          case '9':
            output_char_echo(symbol);
            a=a*10+symbol_to_number(symbol);
            Serial.println(a);
            break;
          
          // Операции
          case 'p': // плюс
          case 'm': // минус
          case 'u': // умножить
          case 'd': // делить
          case 'o': // остаток
          case 'r': // равно
            output_char_echo(symbol);
            // Если предыдущая операция определена, то выполняем её
            switch(operation) {
              case 'p': a=b+a; break;
              case 'm': a=b-a; break;
              case 'u': a=b*a; break;
              case 'd': if(b!=0) a=b/a; else { a=0; error_flag=1; } break;
              case 'o': if(b!=0) a=b%a; else { a=0; error_flag=1; } break;
            }
            if(operation!=0) {
              if(error_flag) {
                output_error();
                error_flag=0;
              } else {
                output_number(a);
              }
            }
            b=a;
            a=0;
            operation=symbol;
            break;
          
          // Сохранение числа в регистр
          case 'a':
            output_char_echo(symbol);
            // Ввод А после равно приводит к сохранению в регистр А
            if(operation=='r') {
              settings.calc_register_a=b;
              settings_save();
            } else {
              // Иначе к загрузке
              a=settings.calc_register_a;
            }
            break;
          case 'b':
            output_char_echo(symbol);
            // Ввод Б после равно приводит к сохранению в регистр Б
            if(operation=='r') {
              settings.calc_register_b=b;
              settings_save();
            } else {
              // Иначе к загрузке
              a=settings.calc_register_b;
            }
            break;
          
          // Сброс
          case ERROR_SYMBOL:
            a=0;
            b=0;
            operation=0;
            error_flag=0;
            break;
          
          // Завершение
          case END_SYMBOL:
            break;
          
          // Прочие символы - ошибка
          default:
            output_error();
            break;
        }
    }
  while(symbol!=END_SYMBOL);
}

// ===============================================
// Устный счёт
// ===============================================

#define INPUT_BUFFER_LEN 10

void mind_count()
{
  int symbol,result;
  int a,b,c,operation;
  int input;
  char state_new;
  char i;
  
  output_char_echo(COUNT_SYMBOL);
  
  state_new=1;
  
  do {
    // Загадываем операцию и выводим её
    if(state_new==1) {
      input=0;
      operation=random(4);
      switch(operation) {
        // Сложение
        case 0:
          a=random(100);
          b=random(100);
          result=a+b;
          output_number(a);
          output_char_normal('p');
          output_number(b);
          break;
        // Вычитание
        case 1:
          a=random(100);
          b=random(100);
          // В случае a<b меняем местами a и b
          if(a<b) {
            c=a;
            a=b;
            b=c;
          }
          result=a-b;
          output_number(a);
          output_char_normal('m');
          output_number(b);
          break;
        // Умножение
        case 2:
          a=random(10);
          b=random(10);
          result=a*b;
          output_number(a);
          output_char_normal('u');
          output_number(b);
          break;
        // Деление
        case 3:
          b=random(10);
          result=random(10);
          a=b*result;
          output_number(a);
          output_char_normal('d');
          output_number(b);
          break;
      }
      
    }
    symbol=input_char();
    switch(symbol) {
      // Ввод числа
      case '0':
      case '1':
      case '2':
      case '3':
      case '4':
      case '5':
      case '6':
      case '7':
      case '8':
      case '9':
        output_char_echo(symbol);
        input=input*10+symbol_to_number(symbol);
        break;
      
      // Ввод сброса
      case ERROR_SYMBOL:
        input=0;
        break;

      // Выход
      case END_SYMBOL:
        break;
      
      // Конец ввода
      case BREAK_SYMBOL:
      case 'r':
        if(input==result) {
          output_win();
        } else {
          output_error();
        }
        state_new=1;
        break;
      
      default:
        output_error();
        break;
    }
  } while(symbol!=END_SYMBOL);
}

// ===============================================
// Таймер
// ===============================================
void timer() {
  long interval=0;
  long start;
  long est;
  int symbol;
  
  output_char_echo(TIMER_SYMBOL);
  do {
    symbol=input_char();
    switch(symbol) {
      case '0':
      case '1':
      case '2':
      case '3':
      case '4':
      case '5':
      case '6':
      case '7':
      case '8':
      case '9':
        output_char_echo(symbol);
        interval=interval*10+symbol_to_number(symbol);
        break;

      // Запустить таймер на указанное значение в минутах
      case 'm':
        // Переводим в секунды
        interval*=60;
      // Запустить таймер на указанное значение в секундах
      case 's':
        // Переводим в миллисекунды
        interval*=1000;
        // Запускаем
        start=millis();
        while((millis()-start)<interval) {
          est=interval-(millis()-start);
          // Если осталось пять секунд - мигаем 5 раз в секунду
          if(est<5000) {
            digitalWrite(LED_PIN, HIGH);
            delay(100);
            digitalWrite(LED_PIN, LOW);
            delay(100);
          // Если осталась минута - мигаем 2 раза в секунду
          } else if(est<60000) {
            digitalWrite(LED_PIN, HIGH);
            delay(200);
            digitalWrite(LED_PIN, LOW);
            delay(200);
          // Иначе мигаем раз в секунду
          } else {
            digitalWrite(LED_PIN, HIGH);
            delay(500);
            digitalWrite(LED_PIN, LOW);
            delay(500);
          }
        }
        output_alarm();
        interval=0;
        break;
      
      case ERROR_SYMBOL:
        return;
        break;
      
      // Выход
      case END_SYMBOL:
        break;
      
      default:
        output_error();
        break;
    }
  } while(symbol!=END_SYMBOL);
}

// ===============================================
// Память
// ===============================================

void memory()
{
  char input_symbols[MEMORY_LEN_MAX];
  char show_symbols[MEMORY_LEN_MAX];
  int i;
  int symbol,result;

  output_char_echo(MEMORY_SYMBOL);
  
  do {
    // Мигаем произвольные MEMORY_LEN символов
    for(i=0;i!=settings.memory_len;i++) {
      show_symbols[i]=output_random(settings.memory_type);
    }
    
    // Пользователь должен повторить
    for(i=0;i!=settings.memory_len;i++) {
      symbol=input_char();
      if(symbol==END_SYMBOL || symbol==ERROR_SYMBOL) return;
      output_char_echo(symbol);
      input_symbols[i]=symbol;
      }
    
    // Смотрим результат
    for(i=0;i!=settings.memory_len;i++) {
      if(input_symbols[i]!=show_symbols[i]) {
        output_error();
        break;
      }
    }
  } while(symbol!=END_SYMBOL);
}

// ===============================================
// Н-назад
// ===============================================

void n_back() {
  int show_symbols[N_BACK_LEN_MAX];
  int symbol;
  int i;
  int result;

  // Инициализация переменных
  for(i=0;i!=settings.n_back_len;i++) {
    show_symbols[i]=output_random(settings.n_back_type);
  }
  
  do {
    symbol=input_char();
    switch(symbol) {
      case ERROR_SYMBOL: return; break;
      case END_SYMBOL: break;
      default:
        // Если символы не совпадают - сообщаем об ошибке
        if(symbol!=show_symbols[0]) {
          output_error();
        }
        // Сдвигаем массив
        for(i=1;i!=settings.n_back_len;i++) {
          show_symbols[i-1]=show_symbols[i];
        }
        // Добавляем следующий символ
        show_symbols[settings.n_back_len-1]=output_random(settings.n_back_type);
        break;
    }
  } while(symbol!=END_SYMBOL);
}

// ===============================================
// Игра быки и коровы
// ===============================================

void bulls_and_cows()
{
  int i,j; // Счётчики циклов
  int numbers[BULLS_AND_COWS_LEN_MAX]; // Загаданные значения
  int input[BULLS_AND_COWS_LEN_MAX]; // Введённые значения
  int bulls,cows; // Число быков, коров
  char message[BULLS_AND_COWS_BUFF];
  char state_new,index;
  int symbol;

  output_char_echo(BULLS_SYMBOL);
  
  // Инициализируем переменные
  state_new=1;
  index=0;
  
  do {
    // Новая игра
    if(state_new) {
      memset(message,0,BULLS_AND_COWS_BUFF);
      for(i=0;i!=settings.bulls_and_cows_len;i++)
        numbers[i]=random(10);
      state_new=0;
    }
    // Читаем символ
    symbol=input_char();
    switch(symbol) {
      // Добавялем цифру в массив
      case '0':
      case '1':
      case '2':
      case '3':
      case '4':
      case '5':
      case '6':
      case '7':
      case '8':
      case '9':
        output_char_echo(symbol);
        input[index]=symbol_to_number(symbol);
        index++;
        break;
      
      // Ошибка - стираем число
      case ERROR_SYMBOL:
        index=0;
        break;
      
      // Выход
      case END_SYMBOL:
        break;
      
      // Повторить сообщение про быков и коров
      case 'p': output_string(message); break;
      default: output_char_echo(ERROR_SYMBOL); break;
    }
    // Если введено нужное число цифр, то начинаем считать быков и коров
    if(index==settings.bulls_and_cows_len) {
      bulls=0;
      cows=0;
      for(i=0;i!=settings.bulls_and_cows_len;i++) {
        // Быки
        if(input[i]==numbers[i]) bulls++;
        // Коровы
        for(j=0;j!=settings.bulls_and_cows_len;j++) {
          if(i==j) continue;
          if(input[i]==numbers[j]) cows++;
        }
      }
      // Если число быков равно числу цифр - победа
      // Если нет - сообщаем число быков и коров
      if(bulls!=settings.bulls_and_cows_len) {
        sprintf(message,"b%dk%d",bulls,cows);
        output_string(message);
      } else {
        output_win();
        state_new=1;
      }
    index=0;
    }
  } while(symbol!=END_SYMBOL);
}

// ===============================================
// Игра охота на лис
// ===============================================
void foxes() {
  char foxes_x[FOXES_COUNT_MAX],foxes_y[FOXES_COUNT_MAX]; // Координаты лис
  char foxes_f[FOXES_COUNT_MAX]; // Флаг найденных лис
  int foxes_around; // Счётчик лис в округе
  int foxes_found; // Счётчик найденных лис
  int symbol;
  int x,y; // Координаты
  int i;
  char state_new,state_win; // Состояния
  char message[FOXES_BUFF]; // Буфер сообщения
  
  output_char_echo(FOXES_SYMBOL);

  // Инициализация переменных
  state_new=1;
  state_win=0;
  x=-1;
  y=-1;

  // Основной цикл
  do {
    // Случай победы
    if(state_win) {
      output_win();
      state_win=0;
    }
    // Новая игра
    if(state_new) {
      memset(message,0,FOXES_BUFF);
      // Расставляем лис
      for(i=0;i!=settings.foxes_count;i++) {
        foxes_x[i]=random(settings.foxes_field_size);
        foxes_y[i]=random(settings.foxes_field_size);
        foxes_f[i]=0;
      }
      state_new=0;
    }
    
    symbol=input_char();
    switch(symbol) {
      // Ввод координат
      case '0':
      case '1':
      case '2':
      case '3':
      case '4':
      case '5':
      case '6':
      case '7':
      case '8':
      case '9':
        output_char_echo(symbol);
        if(x==-1) x=symbol_to_number(symbol);
        else y=symbol_to_number(symbol);
        break;
      
      // Повтор сообщения
      case 'p':
        output_string(message);
        break;

      // Выход
      case END_SYMBOL:
        break;
      
      // Символ ошибки - сброс ввода
      case ERROR_SYMBOL:
        x=-1;
        y=-1;
        break;
      default:
        output_char_echo(ERROR_SYMBOL);
        break;
    }
    // Если координаты введены - считаем лис
    if(x!=-1 && y!=-1) {
      foxes_found=0;
      foxes_around=0;
      state_win=1;
      for(i=0;i!=settings.foxes_count;i++) {
        // Попадание в лису
        if(foxes_x[i]==x && foxes_y[i]==y) {
          foxes_found++;
          foxes_f[i]=1;
        // Лиса в строке или столбце
        } else if(foxes_x[i]==x || foxes_y[i]==y) {
          foxes_around++;
        // Лиса в диагонали
        } else if((foxes_x[i]-foxes_y[i])==(x-y) || (foxes_x[i]+foxes_y[i])==(x+y)) {
          foxes_around++;
        }
        // Если есть непойманные лисы - игра продолжается
        if(foxes_f[i]==0) state_win=0;
      }
      // Если победа, то начинаем новую игру
      if(state_win) {
        state_new=1;
      } else {
        // Формируем сообщение
        if(foxes_found) {
          sprintf(message,"l%d",foxes_found);
        } else {
          sprintf(message,"%d",foxes_around);
        }
        // Выводим сообщение
        output_string(message);
      }
      // Сбрасываем координаты
      x=-1;
      y=-1;
    }
  } while(symbol!=END_SYMBOL);
}

// ===============================================
// Справка
// ===============================================

const char message_help[] PROGMEM = "\
avtor carev vladimir. \
a aptaym. \
b b\x06ki i korovi. \
z nova\x08 zametka. \
k kal'kul\x08tor. \
n n nazad. \
o ohota na lis. \
p pam\x08t'. \
s spravka. \
\x04 \x04tenie zametok. \
\x07 \x07ho.";

void help()
{
  int length=strlen_P(message_help);
  int pos;
  char symbol;
  for(pos=0;pos!=length;pos++) {
    // Если нажата кнопка - прекращаем вывод
    if(digitalRead(BUTTON_PIN)==LOW) return;
    symbol=pgm_read_byte_near(message_help+pos);
    output_char_normal(symbol);
  }
}

// ===============================================
// Аптайм
// Выдаёт число секунд с момента включения
// ===============================================
void uptime()
{
  long current_time=millis();
  long seconds=current_time/1000;
  
  // Показываем где мы
  output_char_echo(UPTIME_SYMBOL);
  
  // Мигаем число секунд
  output_number(seconds);
}

// ===============================================
// Эхо
// Выдаёт введённый символ
// ===============================================
void echo()
{
  int symbol;

  output_char_echo(ECHO_SYMBOL);
  
  do {
    symbol=input_char();
    output_char_echo(symbol);
  } while(symbol!=END_SYMBOL);
}

// ===============================================
// Фонарик
// ===============================================
void light() {
  // Сообщаем где мы
  output_char_echo(LIGHT_SYMBOL);
  // Включаем светодиод
  digitalWrite(LED_PIN, HIGH);
  // Если кнопка уже нажата, ждём пока её отпустят
  while(digitalRead(BUTTON_PIN)!=HIGH);
  // Ждём нажатий
  while(digitalRead(BUTTON_PIN)!=LOW);
  // Избегаем дребезга
  delay(200);
}

// ===================================================
// 
// Настройка и основной цикл
// 
// ===================================================

// ===============================================
// Инициализация периферии
// ===============================================
void setup() {
  // Светодиод
  pinMode(LED_PIN, OUTPUT);
  // Динамик
  pinMode(BUZZER_PIN, OUTPUT);
  // Кнопка для ввода
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  
  // Инициализация генератора случайных чисел
  randomSeed(analogRead(0));

#ifdef SERIAL_DUMP
  int i;
  char symbol,prev_symbol;
  // Выводим содержимое EEPROM при включении
  Serial.begin(9600);
  Serial.println(SERIAL_DUMP_GREETING);
  for(i=0;i!=EEPROM_NOTES_LEN;i++) {
    symbol=EEPROM.read(EEPROM_NOTES_OFFSET+i);
    if(symbol!='\0') {
      Serial.print(symbol);
    } else {
      // Выводим только один перевод строки на последовательность нулей
      if(prev_symbol!=symbol) {
        Serial.println("");
      }
    }
    prev_symbol=symbol;
  }
  Serial.println(SERIAL_DUMP_ENDING);
#endif
  // Загружаем настройки
  settings_load();
}

// ===============================================
// Основной цикл
// ===============================================
void loop() {
  while(1) {
    // Обозначаем где мы
    output_char_echo(MENU_SYMBOL);
    
    // Читаем ввод
    int symbol=input_char();
    
    // Действуем
    switch(symbol) {
      // Аптайм
      case UPTIME_SYMBOL: uptime(); break;

      // Заметка
      case NOTE_SYMBOL: note_add(); break;
      // Чтение заметок
      case READ_SYMBOL: note_read(); break;
      // Повтор фразы
      case CYCLE_SYMBOL: cycle(); break;
      
      // Калькулятор
      case CALC_SYMBOL: calculator(); break;
      // Устный счёт
      case COUNT_SYMBOL: mind_count(); break;
      
      // Быки и коровы
      case BULLS_SYMBOL: bulls_and_cows(); break;
      // Охота на лис
      case FOXES_SYMBOL: foxes(); break;
      
      // Тренировка памяти
      case MEMORY_SYMBOL: memory(); break;
      // Н назад
      case NBACK_SYMBOL: n_back(); break;

      // Справка
      case HELP_SYMBOL: help(); break;

      // Эхо
      case ECHO_SYMBOL: echo(); break;
      // Фонарик
      case LIGHT_SYMBOL: light(); break;
      // Таймер
      case TIMER_SYMBOL: timer(); break;
      
      // Несуществующий пункт меню
      default: output_char_echo(ERROR_SYMBOL); break;
    }
  }
}
;
