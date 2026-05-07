/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   functions.h                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: etaquet <etaquet@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/07 01:30:01 by etaquet           #+#    #+#             */
/*   Updated: 2026/05/07 22:12:53 by etaquet          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef FUNCTIONS_H
# define FUNCTIONS_H

#include "../../includes/kernel.h"

int dump(char *argv[]);
void dump_kernel_stack(size_t words);
int echo(unsigned int argc, char *argv[]);
int halt(void);
int reboot(void);
int shutdown(void);
int flags(void);
int paging(void);
int help(void);
int test_malloc(char *argv[]);
int test_free(char *argv[]);
int panic(char *argv[]);
int fault(int argc, char **argv);

#endif // FUNCTIONS_H