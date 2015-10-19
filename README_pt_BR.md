Biblioteca Modbus para ESP8266
==============================

Esta biblioteca permite que seu ESP8266 se comunique através do protocolo Modbus.
O Modbus é um protocolo do tipo mestre-escravo, utilizado em automação industrial,
podendo ser utilizado em outras áreas, como por exemplo, na automação residencial.

O Modbus geralmente utiliza como meio físico as interfaces seriais RS-232 ou RS-485
(quando é chamado Modbus Serial) e TCP/IP via Ethernet ou WiFi (Modbus IP).

Na versão atual a biblioteca permite que o arduino opere como escravo, suportando
o Modbus IP via rede wireless. Para mais informações sobre o Modbus consulte:

http://pt.wikipedia.org/wiki/Modbus
http://www.modbus.org/docs/Modbus_Application_Protocol_V1_1b.pdf
http://www.modbus.org/docs/Modbus_Messaging_Implementation_Guide_V1_0b.pdf

Características
===============

<ul>
<li>Opera como escravo</li>
<li>Suporta Modbus IP (TCP não keep-alive)</li>
<li>Responde todos os tipos de exceção para as funções suportadas</li>
<li>Suporta as seguintes funções Modbus:</li>
<ul>
    <li>0x01 - Read Coils</li>
    <li>0x02 - Read Input Status (Read Discrete Inputs)</li>
    <li>0x03 - Read Holding Registers</li>
    <li>0x04 - Read Input Registers</li>
    <li>0x05 - Write Single Coil</li>
    <li>0x06 - Write Single Register</li>
    <li>0x0F - Write Multiple Coils</li>
    <li>0x10 - Write Multiple Registers</li>
</ul>
</ul>

<b>Observações:</b>

1. Quando se usa Modbus IP o protocolo de transporte é o TCP (porta 502) e a conexão
é finalizada a cada mensagem transmitida, ou seja, não é do tipo keep-alive.

2. Os offsets para acesso aos registradores são baseados em 0. Assim, tenha cuidado
ao configurar seu seu supervisório ou utilitário de teste. Por exempo, no ScadaBR
(http://www.scadabr.com.br) os offsets são baseados em 0, então, um registrador
configurado como 100 na biblioteca será configurado como 100 no ScadaBR. Por outro
lado, no software de teste CAS Modbus Scanner (http://www.chipkin.com/products/software/modbus-software/cas-modbus-scanner/)
os offsets são baseados em 1, logo, um registrador configurado como 100 na biblioteca
deverá ser 101 neste software.

3. No início do arquivo Modbus.h da biblioteca há uma opção para limitar o funcionamento
da mesma às funções de Holding Registers, salvando espaço na memória de programa.
Basta retirar o comentário da seguinte linha:

```
#define USE_HOLDING_REGISTERS_ONLY
```
Dessa forma, somente as seguintes funções são suportadas:
<ul>
    <li>0x03 - Read Holding Registers</li>
    <li>0x06 - Write Single Register</li>
    <li>0x10 - Write Multiple Registers</li>
</ul>

Como utilizar
=============

```
Este README está em desenvolvimento, por enquanto, consulte os exemplos da biblioteca.
```

Contribuições
=============
http://github.com/andresarmento/modbus-esp8266<br>
prof (at) andresarmento (dot) com

Licença
=======

O código neste repositório é licenciado pela BSD New License.
Veja [LICENSE.txt](LICENSE.txt) para mais informações.


