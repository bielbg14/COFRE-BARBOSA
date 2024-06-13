//////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                       _              //
//               _    _       _      _        _     _   _   _    _   _   _        _   _  _   _          //
//           |  | |  || |\| || |\ ||   |\ ||   || || | |  |   || || |\/| || |  || | |   /|    //    
//         ||  ||  |\  | | | | |/ | |   |/ | |   |   |\  ||  || |\  | | |  | | | |_ | | ||   _|   //
//                                                                                       /              //
//////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <stdio.h> // Biblioteca padrão de entrada e saída
#include "freertos/FreeRTOS.h" // Biblioteca FreeRTOS para RTOS
#include "freertos/task.h" // Biblioteca FreeRTOS para gerenciamento de tarefas
#include "esp_log.h" // Biblioteca ESP-IDF para log
#include "ioplaca.h"  // Biblioteca de I/O específica da placa
#include "lcdvia595.h" // Biblioteca para controle de display LCD via 595
#include "driver/adc.h" // Biblioteca para driver ADC
#include "hcf_adc.h" // Biblioteca personalizada de ADC 
#include "MP_hcf.h"  // Biblioteca de controle do motor 
#include "esp_err.h"  // Biblioteca para tratamento de erros da ESP-IDF
// Área das macros
//-----------------------------------------------------------------------------------------------------------------------

#define DESLIGAR_TUDO       saidas&=0b00000000 // Macro para desligar todas as saídas
#define LIGAR_RELE_1        saidas|=0b00000001 // Macro para ligar o relé 1
#define DESLIGAR_RELE_1     saidas&=0b11111110 // Macro para desligar o relé 1
#define LIGAR_RELE_2        saidas|=0b00000010 // Macro para ligar o relé 2
#define DESLIGAR_RELE_2     saidas&=0b11111101 // Macro para desligar o relé 2
#define LIGAR_TRIAC_1       saidas|=0b00000100 // Macro para ligar o triac 1
#define DESLIGAR_TRIAC_1    saidas&=0b11111011 // Macro para desligar o triac 1
#define LIGAR_TRIAC_2       saidas|=0b00001000 // Macro para ligar o triac 2
#define DESLIGAR_TRIAC_2    saidas&=0b11110111 // Macro para desligar o triac 2
#define LIGAR_TBJ_1         saidas|=0b00010000 // Macro para ligar o transistor bipolar de junção (TBJ) 1
#define DESLIGAR_TBJ_1      saidas&=0b11101111 // Macro para desligar o TBJ 1
#define LIGAR_TBJ_2         saidas|=0b00100000 // Macro para ligar o TBJ 2
#define DESLIGAR_TBJ_2      saidas&=0b11011111 // Macro para desligar o TBJ 2
#define LIGAR_TBJ_3         saidas|=0b01000000 // Macro para ligar o TBJ 3
#define DESLIGAR_TBJ_3      saidas&=0b10111111 // Macro para desligar o TBJ 3
#define LIGAR_TBJ_4         saidas|=0b10000000 // Macro para ligar o TBJ 4
#define DESLIGAR_TBJ_4      saidas&=0b01111111 // Macro para desligar o TBJ 4


#define TECLA_1 le_teclado() == '1' // Macro para verificar se a tecla 1 foi pressionada
#define TECLA_2 le_teclado() == '2' // Macro para verificar se a tecla 2 foi pressionada
#define TECLA_3 le_teclado() == '3' // Macro para verificar se a tecla 3 foi pressionada
#define TECLA_4 le_teclado() == '4' // Macro para verificar se a tecla 4 foi pressionada
#define TECLA_5 le_teclado() == '5' // Macro para verificar se a tecla 5 foi pressionada
#define TECLA_6 le_teclado() == '6' // Macro para verificar se a tecla 6 foi pressionada
#define TECLA_7 le_teclado() == '7' // Macro para verificar se a tecla 7 foi pressionada
#define TECLA_8 le_teclado() == '8' // Macro para verificar se a tecla 8 foi pressionada
#define TECLA_0 le_teclado() == '0' // Macro para verificar se a tecla 0 foi pressionada

// Definir macros para os sensores de fim de curso
#define FIM_DE_CURSO_ABERTO  (entradas & 0b00000001) // Suposição: Fim de curso de aberto na entrada 1
#define FIM_DE_CURSO_FECHADO (entradas & 0b00000010) // Suposição: Fim de curso de fechado na entrada 2

// Área de declaração de variáveis e protótipos de funções
//-----------------------------------------------------------------------------------------------------------------------

static const char *TAG = "Placa";
static uint8_t entradas, saidas = 0; //variáveis de controle de entradas e saídas

int ctrl = 0; // Controle para entrada de dígitos
int numero1 = 0; // Armazenar número digitado
int qdig = 0; // Quantidade de dígitos inseridos
int coluna = 0; // Controle de coluna no display LCD
int resul = 0; // Resultado de alguma operação (não usado no código atual)
char operador; // Operador (não usado no código atual)
char tecla; // Tecla pressionada
char mostra[40]; // String para mostrar no display LCD
uint32_t adcvalor = 0; // Valor lido do ADC
int auxilia = 0;  // Auxiliar (não usado no código atual)
int erros = 0; // Contador de erros de senha

// Função sempre ativa(?)
//-----------------------------------------------------------------------------------------------------------------------


// Funções e ramos auxiliares
//-----------------------------------------------------------------------------------------------------------------------





// Programa Principal
//-----------------------------------------------------------------------------------------------------------------------

void app_main(void)
{
    MP_init(); // configura pinos do motor
    // a seguir, apenas informações de console, aquelas notas verdes no início da execução
    ESP_LOGI(TAG, "Iniciando...");
    ESP_LOGI(TAG, "Versão do IDF: %s", esp_get_idf_version());

    /////////////////////////////////////////////////////////////////////////////////////   Inicializações de periféricos (manter assim)
    
    // inicializar os IOs e teclado da placa
    ioinit();      
    entradas = io_le_escreve(saidas); // Limpa as saídas e lê o estado das entradas

    // inicializar o display LCD 
    lcd595_init();
    lcd595_write(1,1,"    Jornada 1   ");
    lcd595_write(2,1," Programa Basico");
    
    // Inicializar o componente de leitura de entrada analógica
    esp_err_t init_result = hcf_adc_iniciar();
    if (init_result != ESP_OK) {
        ESP_LOGE("MAIN", "Erro ao inicializar o componente ADC personalizado");
    }

    // inica motor
    DRV_init(6, 7);

    //delay inicial
    vTaskDelay(1000 / portTICK_PERIOD_MS); 
    lcd595_clear();

    /////////////////////////////////////////////////////////////////////////////////////   Periféricos inicializados
    hcf_adc_ler(&adcvalor);

    if(adcvalor > 350)
       {
           hcf_adc_ler(&adcvalor);
           while(adcvalor > 350)
           {
                hcf_adc_ler(&adcvalor);
                rotacionar_DRV(0, 11, saidas);
           }
      }

    /////////////////////////////////////////////////////////////////////////////////////   Início do ramo principal                    
   while(1)
    {
       hcf_adc_ler(&adcvalor); // Ler valor do ADC
    

       tecla = le_teclado(); // Ler tecla pressionada

        if(tecla>='0' && tecla <='9')
        {
          if(ctrl == 0)
            {
                numero1 = numero1 * 10 + tecla - '0'; // Constrói o número digitado
                qdig = qdig + 1; // Incrementa a quantidade de dígitos
            }
        }

        if(tecla == 'C')
        {
            numero1 = 0; // Reseta número digitado
            qdig = 0; // Reseta quantidade de dígitos
        }

        lcd595_write(1,0, "Digite a senha!"); // Mensagem no display


        if(tecla!= '_')
        {
            sprintf(&mostra[0], "%c", tecla); // Mostrar tecla pressionada
            lcd595_write(2, coluna, &mostra[0]);
            coluna++;
        }

        if(qdig == 0) // Display vazio
        {
            lcd595_write(2,0, "[ ] [ ] [ ] [ ]");
        }
        if(qdig == 1) // Mostrar * para o primeiro dígito
        {
            sprintf(&mostra[0], "[ ] [ ] [ ] [*]");
            lcd595_write(2,0, &mostra[0]);
        }
        if(qdig == 2) // Mostrar * para o segundo dígito
        {
            sprintf(&mostra[0], "[ ] [ ] [*] [*]");
            lcd595_write(2,0, &mostra[0]);
        }
        if(qdig == 3)  // Mostrar * para o terceiro dígito
        {
            sprintf(&mostra[0], "[ ] [*] [*] [*]");
            lcd595_write(2,0, &mostra[0]);
        }
        if(qdig == 4) // Mostrar * para o quarto dígito
        {
            sprintf(&mostra[0], "[*] [*] [*] [*]");
            lcd595_write(2,0, &mostra[0]);

            vTaskDelay(250 / portTICK_PERIOD_MS);
        }

        if(qdig == 4)
        {
            if(numero1 == 1408) // Se a senha digitada for correta
            {
                int interrompe = 10;

                hcf_adc_ler(&adcvalor);
            
                while(adcvalor <= 2800 && interrompe > 0)
                {
                hcf_adc_ler(&adcvalor);

                lcd595_clear();
                lcd595_write(1,0, "COFRE ABERTO!");

                rotacionar_DRV(1, 11, saidas); // Rotacionar motor para abrir o cofre
                
                interrompe--;
                }
                
                if(adcvalor == 2800 || interrompe == 0)
                {
                    vTaskDelay(5000 / portTICK_PERIOD_MS);

                     hcf_adc_ler(&adcvalor);

                    while(adcvalor >= 350 && interrompe < 11)
                    {  
                        hcf_adc_ler(&adcvalor);

                        

                        lcd595_clear();
                        lcd595_write(1,0, "COFRE FECHANDO!");

                        rotacionar_DRV(0, 11, saidas); // Rotacionar motor para fechar o cofre

                        interrompe++;
                    }
                qdig = 0; // Reseta quantidade de dígitos
                numero1 = 0; // Reseta número digitado
                }
            }

        }

            else // Se a senha digitada for incorreta
            {
                lcd595_clear();
                lcd595_write(1,0, "SENHA ERRADA!");
              
                erros = erros + 1;  // Incrementa contador de erros
                qdig = 0; // Reseta quantidade de dígitos
                numero1 = 0; // Reseta número digitado
            }


            if(erros ==3)
            {
                lcd595_clear();
                lcd595_write(2,1, "TENTE DNV");
                vTaskDelay(500 / portTICK_PERIOD_MS); // Delay para exibir mensagem
            }
        
        vTaskDelay(5000 / portTICK_PERIOD_MS); 
        }
           

       
     
        
        vTaskDelay(100 / portTICK_PERIOD_MS);

    


    // caso erro no programa, desliga o módulo ADC
    hcf_adc_limpar(); 
}