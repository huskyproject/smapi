/*
 *  SMAPI; Modified Squish MSGAPI
 *
 *  Squish MSGAPI0 is copyright 1991 by Scott J. Dudley.  All rights reserved.
 *  Modifications released to the public domain.
 *
 *  Use of this file is subject to the restrictions contain in the Squish
 *  MSGAPI0 licence agreement.  Please refer to licence.txt for complete
 *  details of the licencing restrictions.  If you do not find the text
 *  of this agreement in licence.txt, or if you do not have this file,
 *  you should contact Scott Dudley at FidoNet node 1:249/106 or Internet
 *  e-mail Scott.Dudley@f106.n249.z1.fidonet.org.
 *
 *  In no event should you proceed to use any of the source files in this
 *  archive without having accepted the terms of the MSGAPI0 licensing
 *  agreement, or such other agreement as you are able to reach with the
 *  author.
 */

#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "prog.h"
#include "progprot.h"

static char *colon = ":";
static char *slash = "/";

void _fast Parse_NetNode(char *netnode, word * zone, word * net, word * node, word * point)
{
    ParseNN(netnode, zone, net, node, point, FALSE);
}

void _fast ParseNN(char *netnode, word * zone, word * net, word * node, word * point, word all)
{
    char *p;

    p = netnode;

    if (all && point)
    {
        *point = POINT_ALL;
    }

    if (all && toupper(*netnode) == 'W')
    {
        /* World */

        if (zone)
        {
            *zone = ZONE_ALL;
        }

        if (net)
        {
            *net = NET_ALL;
        }

        if (node)
        {
            *node = NODE_ALL;
        }

        return;
    }


    /* if we have a zone (and the caller wants the zone to be passed back) */

    if (strchr(netnode, ':'))
    {
        if (zone)
        {
            if (all && toupper(*p) == 'A')
            {
                /* All */
                *zone = ZONE_ALL;
            }
            else
            {
                *zone = (word) atoi(p);
            }
        }

        p = firstchar(p, colon, 2);
    }

    /* if we have a net number */

    if (p && *p)
    {
        if (strchr(netnode, '/'))
        {
            if (net)
            {
                if (all && toupper(*p) == 'A')
                {
                    /* All */
                    *net = NET_ALL;
                }
                else
                {
                    *net = (word) atoi(p);
                }
            }

            p = firstchar(p, slash, 2);
        }
        else if (all && toupper(*p) == 'A')
        {
            /* If it's in the form "1:All" or "All" */

            if (strchr(netnode, ':') == NULL && zone)
            {
                *zone = ZONE_ALL;
            }

            *net = NET_ALL;
            *node = NODE_ALL;
            p += 3;
        }
    }

    /* If we got a node number... */

    if (p && *p && node && *netnode != '.')
    {
        if (all && toupper(*p) == 'A')
        {
            /* All */

            *node = NODE_ALL;

            /* 1:249/All implies 1:249/All.All too... */

            if (point && all)
            {
                *point = POINT_ALL;
            }
        }
        else
        {
            *node = (word) atoi(p);
        }
    }

    if (p)
    {
        while (*p && isdigit((int)(*p)))
        {
            p++;
        }
    }

    /* And finally check for a point number... */

    if (p && *p == '.')
    {
        p++;

        if (point)
        {
            if (!p && *netnode == '.')
            {
                p = netnode + 1;
            }

            if (p && *p)
            {
                *point = (word) atoi(p);

                if (all && toupper(*p) == 'A')
                {
                    /* All */
                    *point = POINT_ALL;
                }
            }
            else
            {
                *point = 0;
            }
        }
    }
}
