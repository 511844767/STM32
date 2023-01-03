#ifndef GPIO_SPI_H_CONF__
#define GPIO_SPI_H_CONF__

#define SPI_MISO_GPIO   GPIOB
#define SPI_MISO_PIN    GPIO_Pin_2
#define SPI_MISO_Periph RCC_APB2Periph_GPIOB

#define SPI_MOSI_GPIO   GPIOF
#define SPI_MOSI_PIN    GPIO_Pin_9
#define SPI_MOSI_Periph RCC_APB2Periph_GPIOF

#define SPI_CS_GPIO     GPIOF
#define SPI_CS_PIN      GPIO_Pin_11
#define SPI_CS_Periph RCC_APB2Periph_GPIOF

#define SPI_CLK_GPIO    GPIOB
#define SPI_CLK_PIN     GPIO_Pin_1
#define SPI_CLK_Periph  RCC_APB2Periph_GPIOB

#endif