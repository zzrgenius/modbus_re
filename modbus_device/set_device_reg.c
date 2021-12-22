/*
 * set_device_reg.c
 *
 *  Created on: Dec 22, 2021
 *      Author: zirun
 */

 #include <string.h>
#include <stdio.h>
#include "shell.h"

#if 0
/**
 * @brief shell设置变量
 *
 * @param name 变量名
 * @param value 变量值
 * @return int 返回变量值
 */
int shellSetVar(char *name, int value)
{
    Shell *shell = shellGetCurrent();
    if (shell == NULL)
    {
        return 0;
    }
    ShellCommand *command = shellSeekCommand(shell,
                                             name,
                                             shell->commandList.base,
                                             0);
    if (!command)
    {
        shellWriteString(shell, shellText[SHELL_TEXT_VAR_NOT_FOUND]);
        return 0;
    }
    if (command->attr.attrs.type < SHELL_TYPE_VAR_INT
        || command->attr.attrs.type > SHELL_TYPE_VAR_NODE)
    {
        shellWriteString(shell, name);
        shellWriteString(shell, shellText[SHELL_TEXT_NOT_VAR]);
        return 0;
    }
    return shellSetVarValue(shell, command, value);
}
SHELL_EXPORT_CMD(
SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_FUNC)|SHELL_CMD_DISABLE_RETURN,
setVar, shellSetVar, set var);
#endif
int setreg(int argc, char *agrv[])
{
    printf("%d parameter(s)\r\n", argc);
    for (char i = 1; i < argc; i++)
    {
        printf("%s\r\n",  agrv[i]);
    }
    return 0;
}
