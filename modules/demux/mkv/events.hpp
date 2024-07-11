/*****************************************************************************
 * events.hpp : matroska demuxer
 *****************************************************************************
 * Copyright (C) 2003-2004 VLC authors and VideoLAN
 *
 * Authors: Laurent Aimar <fenrir@via.ecp.fr>
 *          Steve Lhomme <steve.lhomme@free.fr>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2.1 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston MA 02110-1301, USA.
 *****************************************************************************/

#ifndef VLC_MKV_DEMUX_EVENTS_HPP
#define VLC_MKV_DEMUX_EVENTS_HPP

#include <vlc_common.h>
#include <vlc_threads.h>
#include <vlc_actions.h>
#include <vlc_mouse.h>

#include <list>

struct vlc_spu_highlight_t;

namespace mkv {

struct demux_sys_t;
struct pci_t;

class event_thread_t
{
public:
    event_thread_t(demux_t *);
    virtual ~event_thread_t();

    void SetPci(const pci_t *data);
    void ResetPci();
    int SendEventNav( demux_query_e );
    void SetHighlight( vlc_spu_highlight_t & spu_hl );

    bool AddTrack( mkv_track_t & );
    void DelTrack( mkv_track_t & );

private:
    struct ESInfo {
        ESInfo( mkv_track_t & track_, event_thread_t& owner )
            : track( track_ )
            , owner( owner )
        {
            vlc_mouse_Init( &mouse_state );
        }

        bool operator==( const mkv_track_t & t ) const {
            return track.p_es == t.p_es;
        }

        mkv_track_t & track;
        event_thread_t& owner;
        vlc_mouse_t mouse_state;
    };

    struct EventInfo {
        enum {
            ESMouseEvent,
            ActionEvent,
        } type;

        EventInfo( const vlc_mouse_t & state_old, const vlc_mouse_t & state_new )
            : type( ESMouseEvent )
            , mouse{ state_old, state_new }
        {
        }

        EventInfo( NavivationKey key )
            : type( ActionEvent )
            , nav{ key }
        {
        }

        union {
            struct {
                const vlc_mouse_t state_old;
                const vlc_mouse_t state_new;
            } mouse;

            struct {
                const NavivationKey key;
            } nav;
        };
    };

    void EventThread();
    static void *EventThread(void *);

    static void EventMouse( vlc_mouse_t const* state, void* userdata );

    void HandleKeyEvent( EventInfo const& );
    void HandleMouseEvent( EventInfo const& );

    demux_t      *p_demux;

    bool         is_running;
    vlc_thread_t thread;

    vlc_mutex_t  lock;
    vlc_cond_t   wait;
    bool         b_abort;

    typedef std::list<ESInfo> es_list_t;
    es_list_t es_list;

    typedef std::list<EventInfo> pending_events_t;
    pending_events_t pending_events;

private:
    void HandleKeyEvent( NavivationKey key );
    void HandleMousePressed( unsigned x, unsigned y );
};
} // namespace

#endif
