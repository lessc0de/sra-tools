/*===========================================================================
*
*                            PUBLIC DOMAIN NOTICE
*               National Center for Biotechnology Information
*
*  This software/database is a "United States Government Work" under the
*  terms of the United States Copyright Act.  It was written as part of
*  the author's official duties as a United States Government employee and
*  thus cannot be copyrighted.  This software/database is freely available
*  to the public for use. The National Library of Medicine and the U.S.
*  Government have not placed any restriction on its use or reproduction.
*
*  Although all reasonable efforts have been taken to ensure the accuracy
*  and reliability of the software and data, the NLM and the U.S.
*  Government do not and cannot warrant the performance or results that
*  may be obtained by using this software or data. The NLM and the U.S.
*  Government disclaim all warranties, express or implied, including
*  warranties of performance, merchantability or fitness for any particular
*  purpose.
*
*  Please cite the author in any work or product based on this material.
*
* ===========================================================================
*
*/

#ifndef _hpp_general_writer_
#define _hpp_general_writer_

#ifndef _h_general_writer_
#include "general-writer.h"
#endif

#include <iostream>
#include <string>
#include <stdint.h>
#include <vector>
#include <fstream>
#include <map>

#include <string.h>

namespace ncbi
{
#if GW_CURRENT_VERSION == 1
    typedef :: gwp_evt_hdr_v1 gwp_evt_hdr;
#else
#error "unrecognized GW version"
#endif

    class GeneralWriter
    {
    public:

        // ask the general-loader to use this when naming its output
        void setRemotePath ( const std :: string & remote_db );

        // tell the general-loader to use this pre-defined schema
        void useSchema ( const std :: string & schema_file_name,
                         const std :: string & schema_db_spec );

        // add a new table
        // the table-id is returned
        int addTable ( const std :: string &table_name );

        // add a column to an existing table
        // the column-id is returned
        int addColumn ( int table_id, const std :: string &column_name, uint32_t elem_bits, uint8_t flags = 0 );

        // when the column is known to have integer data, use this method
        // it will utilize payload packing for reduced bandwidth
        inline int addIntegerColumn ( int table_id, const std :: string &column_name, uint32_t elem_bits )
        { return addColumn ( table_id, column_name, elem_bits, 1 ); }

        // ensure there are atleast one table and one column
        // set GeneralWriter to open state
        // write out open_stream event header
        void open ();

        // generates a chunk of cell data
        // MUST be entire default value in one event
        void  columnDefault ( int stream_id, uint32_t elem_bits, const void *data, uint32_t elem_count );

        // generate a chunk of cell data
        // may be repeated as often as necessary to complete a single cell's data
        void write ( int stream_id, uint32_t elem_bits, const void *data, uint32_t elem_count );

        // generate an event
        void nextRow ( int table_id );

        // repeat the last row written
        void repeatRow ( uint32_t table_id, uint64_t repeat_count );

        // indicate some sort of exception
        void logError ( const std :: string & msg );

        // generates an end event
        // puts object into state that will reject any further transmissions
        void endStream ();

        // out_fd writes to an open file descriptor ( generally stdout )
        // out_path initializes output stream for writing to a file
        GeneralWriter ( int out_fd );
        GeneralWriter ( const std :: string & out_path );

        // output stream is flushed and closed
        ~ GeneralWriter ();

    private:

        void writeHeader ();
        void internal_write ( const void *data, size_t num_bytes );
        void write_event ( const gwp_evt_hdr * evt, size_t evt_size );

        struct int_stream
        {
            bool operator < ( const int_stream &s ) const;
            int_stream ( int table_id, const std :: string &column_name, uint32_t elem_bits, uint8_t flag_bits );

            int table_id;
            std :: string column_name;
            uint32_t elem_bits;
            uint8_t flag_bits;
        };

        std :: ofstream out;

        std :: map < std :: string, int > table_name_idx;
        std :: map < int_stream, int > column_name_idx;

        std :: vector < int_stream > streams;
        std :: vector < std :: string > table_names;

        uint64_t evt_count;
        uint64_t byte_count;

        uint8_t * packing_buffer;

        int out_fd;

        enum stream_state
        {
            uninitialized,
            header_written,
            remote_name_sent,
            schema_sent,
            remote_name_and_schema_sent,
            have_table,
            have_column,
            opened,
            mid_row,
            closed,
            error
        };
        stream_state state;
    };
}

#endif // _hpp_general_writer_
