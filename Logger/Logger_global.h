/**
    * @copyright   Copyright © 2021-2025 Daniil Budnik. All rights reserved.\n
    * @author      Daniil Budnik. Contacts: <daniil.budnik@gmail.com> <daniil.budnik@pc-set.ru> <daniil.budnik@mail.ru>
*/

#pragma once
#include <QtCore/qglobal.h>

#if defined(LOGGER_LIBRARY)
#  define LOGGER_EXPORT Q_DECL_EXPORT
#else
#  define LOGGER_EXPORT Q_DECL_IMPORT
#endif
