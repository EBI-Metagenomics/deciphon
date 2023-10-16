#ifndef CONTAINER_H
#define CONTAINER_H

/**
 * container_of - cast a member of a structure out to the containing
 * structure
 * @ptr:	the pointer to the member.
 * @type:	the type of the container struct this is embedded in.
 * @member:	the name of the member within the struct.
 *
 */
#define container_of(ptr, type, member)                                        \
  ({                                                                           \
    char *__mptr = (char *)(ptr);                                              \
    ((type *)(__mptr - offsetof(type, member)));                               \
  })

/**
 * container_of_safe - cast a member of a structure out to the containing
 * structure
 * @ptr:	the pointer to the member.
 * @type:	the type of the container struct this is embedded in.
 * @member:	the name of the member within the struct.
 *
 * Return NULL if ptr is NULL.
 */
#define container_of_safe(ptr, type, member)                                   \
  ({                                                                           \
    char *__mptr = (char *)(ptr);                                              \
    __mptr == NULL ? (type *)__mptr                                            \
                   : ((type *)(__mptr - offsetof(type, member)));              \
  })

#endif
