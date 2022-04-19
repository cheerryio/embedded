/*=====================================================================================
 * Description:
 *   https://thoughts.aliyun.com/workspaces/61e270a35d5449001aeacd12/docs/620a13322c4c490001c4aedd
 *    
 *====================================================================================*/
#ifndef _DEVICE_PARAMS
#define _DEVICE_PARAMS

// This file defines structure of modbus parameters which reflect correspond modbus address space
// for each modbus register type (coils, discreet inputs, holding registers, input registers)

// This file defines structure of modbus parameters which reflect correspond modbus address space
// for each modbus register type (coils, discreet inputs, holding registers, input registers)
#pragma pack(push, 1)
typedef struct
{
    uint8_t discrete_input0:1;
    uint8_t discrete_input1:1;
    uint8_t discrete_input2:1;
    uint8_t discrete_input3:1;
    uint8_t discrete_input4:1;
    uint8_t discrete_input5:1;
    uint8_t discrete_input6:1;
    uint8_t discrete_input7:1;
    uint8_t discrete_input8:1;
    uint8_t discrete_input9:1;
    uint8_t discrete_input10:1;
    uint8_t discrete_input11:1;
    uint8_t discrete_input12:1;
    uint8_t discrete_input13:1;
    uint8_t discrete_input14:1;
    uint8_t discrete_input15:1;

} discrete_reg_params_t;
#pragma pack(pop)

#pragma pack(push, 1)
typedef struct
{
    uint8_t coils_output0:1;
    uint8_t coils_output1:1;
    uint8_t coils_output2:1;
    uint8_t coils_output3:1;
    uint8_t coils_output4:1;
    uint8_t coils_output5:1;
    uint8_t coils_output6:1;
    uint8_t coils_output7:1;
    uint8_t coils_output8:1;
    uint8_t coils_output9:1;
    uint8_t coils_output10:1;
    uint8_t coils_output12:1;
    uint8_t coils_output13:1;
    uint8_t coils_output14:1;
    uint8_t coils_output15:1;
} coil_reg_params_t;
#pragma pack(pop)

//3x dio16没有用。
#pragma pack(push, 1)
typedef struct
{
    uint16_t DI_A;
    uint16_t DI_B;
} input_reg_params_t;
#pragma pack(pop)

//4x 寄存器区定义支持32bit，本品只有DO_A和DI_A各16bit.
#pragma pack(push, 1)
typedef struct
{
    uint16_t DO_A;
    uint16_t DO_B;
    uint16_t DI_A;
    uint16_t DI_B;
} holding_reg_params_t;
#pragma pack(pop)

extern holding_reg_params_t holding_reg_params;
extern input_reg_params_t input_reg_params;
extern coil_reg_params_t coil_reg_params;
extern discrete_reg_params_t discrete_reg_params;


// Defines below are used to define register start address for each type of Modbus registers
#define MB_REG_COILS_START                  (0x20)
#define MB_REG_DISCRETE_INPUT_START         (0x40)
#define MB_REG_INPUT_START                  (0x0000)   
#define MB_REG_HOLDING_START                (0x20)


#endif // !defined(_DEVICE_PARAMS)
