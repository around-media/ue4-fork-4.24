// Copyright 2021 fpwong. All Rights Reserved.

#pragma once

#if ENGINE_MINOR_VERSION >= 25 || ENGINE_MAJOR_VERSION >= 5
#define BA_PROPERTY FProperty
#define BA_FIND_FIELD FindUField
#define BA_FIND_PROPERTY FindFProperty
#define BA_WEAK_FIELD_PTR TWeakFieldPtr
#else
#define BA_PROPERTY UProperty
#define BA_FIND_FIELD FindField
#define BA_FIND_PROPERTY FindField
#define BA_WEAK_FIELD_PTR TWeakObjectPtr
#endif
