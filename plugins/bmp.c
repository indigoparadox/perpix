
#include <perpix.h>

#define BMP_HEADER_SZ 40

#define BMPINFO_OFFSET_WIDTH 4
#define BMPINFO_OFFSET_HEIGHT 8
#define BMPINFO_OFFSET_COLOR_PLANES 12
#define BMPINFO_OFFSET_BPP 14
#define BMPINFO_OFFSET_CMP 16
#define BMPINFO_OFFSET_SZ 20
#define BMPINFO_OFFSET_HRES 24
#define BMPINFO_OFFSET_VRES 28
#define BMPINFO_OFFSET_PAL_SZ 32
#define BMPINFO_OFFSET_IMP_COLORS 36

#define BMP_OPTS_FLAG_UPSIDE_DOWN 0x01

#define BMP_COMPRESSION_NONE (0)

#if 0
int32_t bmp_verify_opts( struct CONVERT_OPTIONS* o ) {
   return 1;
}

int32_t bmp_grid_sz(
   const struct CONVERT_GRID* grid, struct CONVERT_OPTIONS* o
) {
   int32_t bmp_row_sz = 0,
      palette_entries = 0,
      bmp_buffer_sz = 0;

   /* Add rows padded out to 4 bytes. */
   bmp_row_sz = (grid->sz_x * o->bpp) / 8;
   if( 0 != bmp_row_sz % 4 ) {
      bmp_row_sz += (4 - (bmp_row_sz % 4));
   }
   o->bmp_data_sz = (grid->sz_y * bmp_row_sz);

   palette_entries = bmp_colors_count( o->bpp );

   bmp_buffer_sz = 
      sizeof( struct BITMAP_FILE_HEADER ) +
      sizeof( struct BITMAP_DATA_HEADER ) +
      (4 * palette_entries) + /* Palette entries are 32-bit (4 bytes). */
      o->bmp_data_sz;

   return bmp_buffer_sz;
}

/**
 * \return The number of bytes successfully written.
 */
int32_t bmp_write_file(
   const char* path, const struct CONVERT_GRID* grid, struct CONVERT_OPTIONS* o
) {
   int32_t bmp_buffer_sz = 0,
      bmp_file_sz = 0,
      written = 0;
   uint8_t* bmp_buffer = NULL;
   FILE* file_out = NULL;

   bmp_buffer_sz = bmp_grid_sz( grid, o );

   /* TODO: Use memory architecture. */
   bmp_buffer = calloc( 1, bmp_buffer_sz );
   assert( NULL != bmp_buffer );

   bmp_file_sz = bmp_write( bmp_buffer, bmp_buffer_sz, grid, o );
   assert( bmp_file_sz == bmp_buffer_sz );
   if( 0 >= bmp_file_sz ) {
      free( bmp_buffer );
      return bmp_file_sz;
   }

   file_out = fopen( path, "wb" );
   assert( NULL != file_out );

   written = fwrite( bmp_buffer, 1, bmp_buffer_sz, file_out );

   fclose( file_out );
   free( bmp_buffer );

   assert( written == bmp_buffer_sz );
   if( written != bmp_buffer_sz ) {
      return CONVERT_ERROR_FILE_WRITE;
   }

   return bmp_buffer_sz;
}

static uint8_t bmp_palette_cga_4_to_16( int color_in ) {
   switch( color_in ) {
   case 0:
      return 0;

   case 1:
      /* Cyan */
      return 11;

   case 2:
      /* Magenta */
      return 13;

   case 3:
      /* White */
      return 15;
   }

   return 0;
}

static uint8_t bmp_palette_any_to_mono( uint8_t color_in, uint8_t reverse ) {
   if( reverse ) {
      /* In reverse, 0s become 1s. */
      return 0 == color_in ? 1 : 0;
   } else {
      return 0 == color_in ? 0 : 1;
   }
   return 0;
}

uint8_t bmp_colors_count( uint8_t bpp ) {
   switch( bpp ) {
   case 1:
      return 2;

   case 2:
      return 4;

   case 4:
   default:
      return 16;
   }
}

/**
 * \return The number of bytes successfully written.
 */
int32_t bmp_write(
   uint8_t* buf_ptr, uint32_t buf_sz,
   const struct CONVERT_GRID* grid, struct CONVERT_OPTIONS* o
) {
   int32_t x = 0,
      y = 0,
      bit_idx = 0;
   uint32_t i = 0,
      row_bytes = 0,
      bmp_file_byte_idx = 0;
   uint8_t byte_buffer = 0,
      bit_mask_in = 0,
      bit_mask_out = 0;
   uint32_t bmp_colors = 0,
      * bmp_palette = 0;
   struct BITMAP_FILE_HEADER* file_header = NULL;
   struct BITMAP_DATA_HEADER* data_header = NULL;

   if( o->bmp_no_file_header ) {
      file_header = NULL;
      data_header = (struct BITMAP_DATA_HEADER*)buf_ptr;

      bmp_file_byte_idx +=
         sizeof( struct BITMAP_DATA_HEADER );

   } else {
      file_header = (struct BITMAP_FILE_HEADER*)buf_ptr;
      data_header = (struct BITMAP_DATA_HEADER*)&(buf_ptr[
         sizeof( struct BITMAP_FILE_HEADER )]);

      file_header->id[0] = 'B';
      file_header->id[1] = 'M';
         
      bmp_file_byte_idx +=
         sizeof( struct BITMAP_FILE_HEADER ) +
         sizeof( struct BITMAP_DATA_HEADER );
   }

   bmp_colors = bmp_colors_count( o->bpp );

   assert( sizeof( struct BITMAP_DATA_HEADER ) == 40 );
   
   data_header->header_sz = 40;  /* Header Size */
   data_header->bitmap_w = grid->sz_x;
   data_header->bitmap_h = grid->sz_y;
   data_header->planes = 1; /* Planes */
   data_header->bpp = o->bpp;
   data_header->compression = 0; /* Compression. */
   data_header->image_sz = 
      (data_header->bitmap_w * data_header->bitmap_h) / (8 / o->bpp);
   data_header->hres = 72; /* HRes */
   data_header->vres = 72; /* VRes */
   data_header->colors = bmp_colors;
   data_header->colors_important = 0; /* Important Colors */

   bmp_palette = (uint32_t*)&(buf_ptr[bmp_file_byte_idx]);
   *bmp_palette = 0x00000000; /* Palette: Black */
   bmp_palette++;
   bmp_file_byte_idx += 4;
   
   switch( o->bpp ) {
   case 1:
      *bmp_palette = 0x00ffffff; /* Palette: White */
      bmp_palette++;
      bmp_file_byte_idx += 4;

      assert( bmp_file_byte_idx ==
         (o->bmp_no_file_header ? 0 : sizeof( struct BITMAP_FILE_HEADER )) +
         sizeof( struct BITMAP_DATA_HEADER ) +
         (2 * 4) );
      break;

   case 2:
      *bmp_palette = 0x0000ffff; /* Palette: Cyan */
      bmp_palette++;
      *bmp_palette = 0x00ff00ff; /* Palette: Magenta */
      bmp_palette++;
      *bmp_palette = 0x00ffffff; /* Palette: White */
      bmp_palette++;
      bmp_file_byte_idx += (4 * 3);

      assert( bmp_file_byte_idx ==
         (o->bmp_no_file_header ? 0 : sizeof( struct BITMAP_FILE_HEADER )) +
         sizeof( struct BITMAP_DATA_HEADER ) +
         (4 * 4) );
      break;

   case 4:
   default:
      *bmp_palette = dio_reverse_endian_32( 0xaa000000 );
      bmp_palette++;
      *bmp_palette = dio_reverse_endian_32( 0x00aa0000 );
      bmp_palette++;
      *bmp_palette = dio_reverse_endian_32( 0xaaaa0000 );
      bmp_palette++;
      *bmp_palette = dio_reverse_endian_32( 0x0000aa00 );
      bmp_palette++;
      *bmp_palette = dio_reverse_endian_32( 0xaa00aa00 );
      bmp_palette++;
      *bmp_palette = dio_reverse_endian_32( 0x0055aa00 );
      bmp_palette++;
      *bmp_palette = dio_reverse_endian_32( 0xaaaaaa00 );
      bmp_palette++;
      *bmp_palette = dio_reverse_endian_32( 0x55555500 );
      bmp_palette++;
      *bmp_palette = dio_reverse_endian_32( 0xff555500 );
      bmp_palette++;
      *bmp_palette = dio_reverse_endian_32( 0x55ff5500 );
      bmp_palette++;
      *bmp_palette = dio_reverse_endian_32( 0xffff5500 );
      bmp_palette++;
      *bmp_palette = dio_reverse_endian_32( 0x5555ff00 );
      bmp_palette++;
      *bmp_palette = dio_reverse_endian_32( 0xff55ff00 );
      bmp_palette++;
      *bmp_palette = dio_reverse_endian_32( 0x55ffff00 );
      bmp_palette++;
      *bmp_palette = dio_reverse_endian_32( 0xffffff00 );
      bmp_palette++;
      bmp_file_byte_idx += (4 * 15);

      assert( bmp_file_byte_idx ==
         (o->bmp_no_file_header ? 0 : sizeof( struct BITMAP_FILE_HEADER )) +
         sizeof( struct BITMAP_DATA_HEADER ) +
         (16 * 4) );

      break;
   }

   /* Store the offset of the start of the bitmap data. */
   if( !o->bmp_no_file_header ) {
      file_header->bmp_offset = bmp_file_byte_idx;
   }
   o->bmp_data_offset_out = bmp_file_byte_idx;

   /* Calculate bit masks. */
   for( i = 0 ; o->bpp > i ; i++ ) {
      bit_mask_out <<= 1;
      bit_mask_out |= 0x01;
   }
   for( i = 0 ; grid->bpp > i ; i++ ) {
      bit_mask_in <<= 1;
      bit_mask_in |= 0x01;
   }

   debug_printf( 1, "using write mask: 0x%x", bit_mask_out );

   for( y = grid->sz_y - 1 ; y >= 0 ; y-- ) {
      row_bytes = 0;
      for( x = 0 ; x < grid->sz_x ; x++ ) {
         assert( y >= 0 );
         assert( x >= 0 );
         assert( (y * grid->sz_x) + x < grid->data_sz );

         /* Format grid data into byte. */
         byte_buffer <<= o->bpp;
         switch( o->bpp ) {
         case 1:
            byte_buffer |= 
               bmp_palette_any_to_mono( grid->data[(y * grid->sz_x) + x],
               o->reverse);
            break;

         case 4:
         default:
            byte_buffer |= 
               bmp_palette_cga_4_to_16( grid->data[(y * grid->sz_x) + x] )
               & bit_mask_out;
            break;
         }
         /* dio_print_binary( byte_buffer ); */
         bit_idx += o->bpp;

         /* Write finished byte. */
         if( 0 != bit_idx && 0 == bit_idx % 8 ) {
            debug_printf( 
               1, "bmp: writing byte %d out of %d (row %d, col %d)",
                  bmp_file_byte_idx, buf_sz, y, x );
            assert( bmp_file_byte_idx < buf_sz );
            buf_ptr[bmp_file_byte_idx] = byte_buffer;
            byte_buffer = 0;
            bmp_file_byte_idx++;
            row_bytes++;
            bit_idx = 0;
         }
      }
      while( 0 != (row_bytes % 4) ) {
         debug_printf( 1, "bmp: adding row padding byte" );
         buf_ptr[bmp_file_byte_idx] = '\0';
         bmp_file_byte_idx++;
         row_bytes++;
      }
   }

   if( !o->bmp_no_file_header ) {
      file_header->file_sz = bmp_file_byte_idx;
   }

   return bmp_file_byte_idx;
}

#endif

#if 0
MPLUG_EXPORT MERROR_RETVAL bmp_px_sz( struct PERPIX_PLUG_ENV* plug_env ) {
   MERROR_RETVAL retval = MERROR_OK;
   struct PERPIX_GRID* grid =
      &(plug_env->grid_pack->layers[plug_env->layer_idx]);

   if( plug_env->layer_idx >= plug_env->grid_pack->count ) {
      error_printf( "invalid grid pack layer selected: " UPRINTF_U32_FMT,
         plug_env->layer_idx );
      retval = MERROR_OVERFLOW;
      goto cleanup;
   }

   plug_env->layer_sz = (
      ((((grid->bpp * grid->w) + 31) / 32) * 4) * grid->h);

cleanup:
   return retval;
}
#endif

MPLUG_EXPORT MERROR_RETVAL bmp_read_info_header(
   struct PERPIX_PLUG_ENV* plug_env
) {
   MERROR_RETVAL retval = MERROR_OK;
   int bmp_compression = 0,
      hdr_sz = 0;
   struct PERPIX_GRID* grid = NULL;
   uint32_t bmp_w = 0,
      bmp_h = 0;
   uint16_t bmp_bpp = 0;
   uint32_t bmp_ncolors = 0;

   debug_printf( 3, "bmp plugin started header..." );

   if( NULL != plug_env->test_grid ) {
      grid = plug_env->test_grid;
   } else {
      /* Make sure our desired layer exists! */
      if( plug_env->layer_idx >= plug_env->grid_pack->count ) {
         error_printf( "invalid grid pack layer selected: " UPRINTF_U32_FMT,
            plug_env->layer_idx );
         retval = MERROR_OVERFLOW;
         goto cleanup;
      }

      /* Select our desired layer. */
      grid = &(plug_env->grid_pack->layers[plug_env->layer_idx]);
   }

   /* Read the bitmap image header. */
   hdr_sz = perpix_read_lsbf_32( plug_env->buf, 0 );
   if( 40 != hdr_sz ) { /* Windows BMP. */
      error_printf( "invalid header size: %u", hdr_sz );
      retval = MERROR_FILE;
      goto cleanup;
   }
   debug_printf( 2, "bitmap header is %u bytes", hdr_sz );

   /* Read bitmap image dimensions. */
   bmp_w = perpix_read_lsbf_32( plug_env->buf, 4 );
   bmp_h = perpix_read_lsbf_32( plug_env->buf, 8 );
   if( 0 > grid->h ) {
      /* Note that the bitmap is upside down! */
      debug_printf( 2, "bitmap is upside down: " UPRINTF_S32_FMT, bmp_h );
      plug_env->flags |= BMP_OPTS_FLAG_UPSIDE_DOWN;
      bmp_h *= -1;
   }

   if( NULL != plug_env->test_grid ) {
      /* We're getting dimensions, so pass them back using the grid. */
      grid->w = bmp_w;
      grid->h = bmp_h;
   } else {
      /* We've been passed a valid grid, so make sure the image fits! */
      if( bmp_w != grid->w /* XXX || bmp_h != grid->h */ ) {
         error_printf( "passed grid has incompatible size! (has %u x %u, "
            "needs %u x %u)", grid->w, grid->h, bmp_w, bmp_h );
         retval = MERROR_OVERFLOW;
         goto cleanup;
      }
   }

   /* Check that we're a palettized image. */
   plug_env->bpp = perpix_read_lsbf_16( plug_env->buf, 14 );
   if( 8 < plug_env->bpp ) {
      error_printf( "invalid bitmap bpp: %u", plug_env->bpp );
      retval = MERROR_FILE;
      goto cleanup;
   }

   /* Make sure there's no weird compression. */
   bmp_compression = perpix_read_lsbf_16( plug_env->buf, 16 );
   if( BMP_COMPRESSION_NONE != bmp_compression ) {
      error_printf( "invalid bitmap compression: %u", bmp_compression );
      retval = MERROR_FILE;
      goto cleanup;
   }

   bmp_ncolors = perpix_read_lsbf_16( plug_env->buf, 32 );
   if( NULL != plug_env->test_grid ) {
      /* We're getting dimensions, so pass them back using the grid. */
      grid->palette_ncolors = bmp_ncolors;
   } else {
      /* We've been passed a valid grid, so make sure the image fits! */
      if( bmp_ncolors != grid->palette_ncolors ) {
         error_printf( "passed grid has incompatible colors! (has %u, "
            "needs %u)", grid->palette_ncolors, bmp_ncolors );
         retval = MERROR_OVERFLOW;
         goto cleanup;
      }
   }

   debug_printf( 2, "bitmap is %d x %d, %u bpp (palette has %u colors)",
      grid->w, grid->h, plug_env->bpp, grid->palette_ncolors );

cleanup:

   return retval;
}

MPLUG_EXPORT MERROR_RETVAL bmp_read_palette(
   struct PERPIX_PLUG_ENV* plug_env
) {
   MERROR_RETVAL retval = MERROR_OK;
   size_t i = 0;
   uint32_t* p_palette = NULL;
   struct PERPIX_GRID* grid = 
      &(plug_env->grid_pack->layers[plug_env->layer_idx]);

   if( plug_env->layer_idx >= plug_env->grid_pack->count ) {
      error_printf( "invalid grid pack layer selected: " UPRINTF_U32_FMT,
         plug_env->layer_idx );
      retval = MERROR_OVERFLOW;
      goto cleanup;
   }

   debug_printf( 2, "setting up palette for layer: " UPRINTF_U32_FMT,
      plug_env->layer_idx );

   p_palette = grid_palette( grid );

   for( i = 0 ; grid->palette_ncolors > i ; i++ ) {
      if( i * 4 > plug_env->buf_sz ) {
         error_printf( "palette overflow!" );
         retval = MERROR_OVERFLOW;
         goto cleanup;
      }
      p_palette[i] = perpix_read_lsbf_32( plug_env->buf, i * 4 );
      debug_printf( 1, "set palette entry " SIZE_T_FMT " to " UPRINTF_X32_FMT,
         i, p_palette[i] );
   }

cleanup:
   return retval;
}

MPLUG_EXPORT MERROR_RETVAL bmp_read_px( struct PERPIX_PLUG_ENV* plug_env ) {
   MERROR_RETVAL retval = MERROR_OK;
   uint32_t x = 0,
      y = 0,
      i = 0,
      byte_idx = 0,
      bit_idx = 0;
   struct PERPIX_GRID* grid = 
      &(plug_env->grid_pack->layers[plug_env->layer_idx]);
   uint8_t* p_grid_px = NULL;
   uint8_t byte_buffer = 0,
      byte_mask = 0,
      pixel_buffer = 0;

   if( plug_env->layer_idx >= plug_env->grid_pack->count ) {
      error_printf( "invalid grid pack layer selected: " UPRINTF_U32_FMT,
         plug_env->layer_idx );
      retval = MERROR_OVERFLOW;
      goto cleanup;
   }

   debug_printf( 3, "bmp plugin started pixels..." );

   /* Figure out where we're writing data to. */
   p_grid_px = grid_px( grid );

   /* TODO: Make sure grid is big enough! */

   /*
   grid->palette_offset = sizeof( struct PERPIX_GRID );
   grid->px_offset = sizeof( struct PERPIX_GRID ) + pal_sz;
   */

   /* Read the bitmap data. */

   /* Grid starts from top, bitmap starts from bottom. */
   /* TODO: Use upside-down flag! */
   y = grid->h - 1;
   while( grid->h > y ) {
      debug_printf( 1, "bmp: byte_idx %u, bit %u (%u), row %d, col %d (%u)",
         byte_idx, bit_idx, plug_env->bpp, y, x, (y * grid->w) + x );

      if( 0 == bit_idx % 8 ) {
         /* Move on to a new byte. */
         byte_buffer = plug_env->buf[byte_idx];
         byte_idx++;
         bit_idx = 0;
         pixel_buffer = 0;

         /* Build a bitwise mask based on the bitmap's BPP. */
         byte_mask = 0;
         for( i = 0 ; plug_env->bpp > i ; i++ ) {
            byte_mask <<= 1;
            byte_mask |= 0x01;
         }
      }

      /* TODO: Bounds checking! */
      assert( (y * sz_x) + x < (buf_sz * (8 / grid->bpp)) );

      debug_printf( 3, "bm: 0x%02x", byte_mask );

      /* Use the byte mask to place the bits for this pixel in the
       * pixel buffer.
       */
      pixel_buffer |= byte_buffer & byte_mask;
      
      /* Shift the pixel buffer so the index lines up at the first bit. */
      pixel_buffer >>= bit_idx;
      debug_printf( 3, "new bit_idx: %u", bit_idx );

      /* Place the pixel buffer at the X/Y in the grid. */
      p_grid_px[(y * grid->w) + x] = pixel_buffer;

      /* Increment the bits position byte mask by the bpp so it's pointing
       * to the next pixel in the bitmap for the next go around.
       */
      byte_mask <<= plug_env->bpp;
      bit_idx += plug_env->bpp;

      /* Move to the next pixel. */
      x++;
      if( x >= grid->w ) {
         /* Move to the next row. */
         y--;
         x = 0;
         while( byte_idx % 4 != 0 ) {
            byte_idx++;
         }
         /* Get past the padding. */
      }
   }

cleanup:
   return retval;
}

MPLUG_EXPORT MERROR_RETVAL bmp_layer_sz( struct PERPIX_PLUG_ENV* plug_env ) {
   /* TODO */
}

MPLUG_EXPORT MERROR_RETVAL bmp_layers( struct PERPIX_PLUG_ENV* plug_env ) {
   return 1; /* Bitmap file only has one layer! */
}

MPLUG_EXPORT MERROR_RETVAL bmp_read( struct PERPIX_PLUG_ENV* plug_env ) {
   MERROR_RETVAL retval = MERROR_OK;
   struct PERPIX_PLUG_ENV hdr_env;
   /* Bitmap *file* only has one layer. */
   struct PERPIX_GRID* grid = &(plug_env->grid_pack->layers[0]);
   uint32_t bmp_data_offset = 0;
   size_t bmp_file_sz = 0;

   if(
      NULL != plug_env->grid_pack && (
         0 == plug_env->grid_pack->version || 1 < plug_env->grid_pack->version
      )
   ) {
      error_printf( "don't know how to write grid version: " UPRINTF_U32_FMT,
         plug_env->grid_pack->version );
      retval = MERROR_FILE;
      goto cleanup;
   }

   debug_printf( 3, "bmp plugin started reading..." );

   memcpy( &hdr_env, plug_env, sizeof( struct PERPIX_PLUG_ENV ) );

   /* Read the bitmap file header. */
   if( 0x42 != plug_env->buf[0] || 0x4d != plug_env->buf[1] ) {
      error_printf(
         "invalid magic number: 0x%02x 0x%02x (looking for 0x%02x 0x%02x)",
         plug_env->buf[0], plug_env->buf[1], 0x42, 0x4d );
      retval = MERROR_FILE;
      goto cleanup;
   }

   bmp_file_sz = perpix_read_lsbf_32( plug_env->buf, 2 );
   if( bmp_file_sz != plug_env->buf_sz ) {
      error_printf(
         "bitmap size field " SIZE_T_FMT " does not match buffer size: "
         SIZE_T_FMT, bmp_file_sz, plug_env->buf_sz );
      retval = MERROR_FILE;
      goto cleanup;
   }

   debug_printf( 1, "buffer size " SIZE_T_FMT ", as expected...",
      plug_env->buf_sz );

   bmp_data_offset = perpix_read_lsbf_32( plug_env->buf, 10 );
   debug_printf( 2, "bitmap data starts at %u bytes", bmp_data_offset );

   hdr_env.buf += 14; /* BMP -file- header size. */
   hdr_env.buf_sz -= 14;
   bmp_data_offset -= 14;
   retval = bmp_read_info_header( &hdr_env );
   maug_cleanup_if_not_ok();

   if(
      PERPIX_PLUG_FLAG_HEADER_ONLY ==
         (plug_env->flags & PERPIX_PLUG_FLAG_HEADER_ONLY)
   ) {
      debug_printf( 2, "header-only flag specified, not reading image!" );
      goto cleanup;
   }

#if 0
   bmp_px_sz( plug_env );
   if(
      plug_env->layer_sz + (grid->palette_ncolors * 4) + 40 + 14 != buf_sz
   ) {
      error_printf(
         "invalid buffer size: " SIZE_T_FMT ", expected: " SIZE_T_FMT
         " (%u rows of %u pixels, each row sized %u bytes for image sized %u"
         ", plus %u colors of 4 bytes each, plus 54 bytes of header)",
         buf_sz, plug_env->layer_sz + 40 + 14,
         grid->h, grid->w, (((grid->bpp * grid->w) + 31) / 32) * 4,
         grid->h * (((grid->bpp * grid->w) + 31) / 32) * 4,
         grid->palette_ncolors * 4 );
      retval = MERROR_FILE;
      goto cleanup;
   }
#endif

   hdr_env.buf += 40; /* BMP -info- header size. */
   hdr_env.buf_sz -= 40;
   bmp_data_offset -= 40;
   retval = bmp_read_palette( &hdr_env );
   maug_cleanup_if_not_ok();

   hdr_env.buf += bmp_data_offset;
   hdr_env.buf_sz -= bmp_data_offset;
   retval = bmp_read_px( &hdr_env );
   maug_cleanup_if_not_ok();

cleanup:

   return retval;
}

