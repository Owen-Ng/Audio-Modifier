#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * It is a format string that will be supplied to fscanf to read from stdin
 */
const char format_fscanf_wrapper[] = "%08x: %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c\n";

/*
 * When full line of data is parsed by fscanf_wrapper, 33 is returned. 16 integers, 16 characters and 1 line number
 */
const int full_line_ret_fscanf = 33;

/*
 * This is a function wrapper for fscanf to save the hassle of writing the function over and over again.
 * The format is hard-coded in this function
 *
 * 
 *  @param *file File to read from
 *  @param *address  Pointer to unsigned integer to read, line number, 'address' to
 *  @param integers[16]  Array of 16 integers, to store bytes to, %02x
 *  @param characters[16]  Array of 16 characters, to store characters to, %c
 * 
 *  return Result of fscanf or -1, if any pointer is NULL
 */
int fscanf_wrapper(FILE *file, unsigned *address, int integers[16], char characters[16])
{
    if (file == NULL || address == NULL || integers == NULL || characters == NULL)
    {
        return -1;
    }

    return fscanf(file, format_fscanf_wrapper,
                  address, &integers[0], &integers[1], &integers[2], &integers[3], &integers[4],
                   &integers[5], &integers[6], &integers[7], &integers[8], &integers[9],
                  &integers[10], &integers[11], &integers[12], &integers[13], &integers[14],
                   &integers[15], &characters[0], &characters[1], &characters[2], &characters[3], &characters[4],
                  &characters[5], &characters[6], &characters[7], &characters[8], &characters[9], &characters[10], 
                  &characters[11], &characters[12], &characters[13], &characters[14],
                  &characters[15]);
}

/*
 * This function will convert integer, read as hex to char, that is printed after
 *
 * 
 * @param integer  The integer to convert
 * 
 * @return The converted char. If conversion is possible, it will return that, else '.' is returned
 */
char to_char(int integer)
{
    /*
     * All printable characters in ASCII table lies between Space and Tilde (~), else we will output '.'
     */
    if (integer >= ' ' && integer <= '~')
    {
        return (char)integer;
    }
    else
    {
        return '.';
    }
}

/*
 * This is a function wrapper to fprintf
 *
 * 
 *  @param *file  The file to write to
 *  @param read_ints  Number of integers read, note this is not what fscanf_wrapper gave
 *  @param address  The line number to write
 *  @param integer[16]  The array of integer that was read, characters will be interpreted from this
 *
 *  @return Result of fprintf or -1, if any pointer is NULL or read_ints <= 0 or read_ints > 16
 */
int fprintf_wrapper(FILE *file, int read_ints, unsigned address, const int integers[16])
{
    int i;
    int result;
    char characters[16];

    if (file == NULL || integers == NULL || read_ints <= 0 || read_ints > 16)
    {
        return -1;
    }

    for (i = 0; i < read_ints; i++)
    {
        characters[i] = to_char(integers[i]);
    }

    result = fprintf(file, "%08x: ", address);
    for (i = 0; i < read_ints; i++)
    {
        result += fprintf(file, "%02x ", integers[i]);
    }

    /*
     * We have to fill up the space of ints that are not present, i.e. the last line
     */
    for (i = read_ints; i < 16; i++)
    {
        result += fprintf(file, "   ");
    }

    for (i = 0; i < read_ints; i++)
    {
        result += fprintf(file, "%c", characters[i]);
    }

    result += fprintf(file, "\n");
    return result;
}

/*
 * This function will check header of wav, from first reading using fscanf_wrapper
 *
 * 
 * @param integers[16]  Array of integer read
 * @param characters[16]  Array of char read
 * 
 * @return 1 if okay, else 0
 */
int first_read_check(const int integers[16], const char characters[16])
{
    if (integers == NULL || characters == NULL)
    {
        return 0;
    }

    if (characters[0] != 'R' || characters[1] != 'I' || characters[2] != 'F' || characters[3] != 'F')
    {
        return 0;
    }

    if (characters[8] != 'W' || characters[9] != 'A' || characters[10] != 'V' || characters[11] != 'E' ||
            characters[12] != 'f' || characters[13] != 'm' || characters[14] != 't' || characters[15] != ' ')
    {
        return 0;
    }

    return 1;
}

/*
 * This function will extract information from header of wav, from second reading using fscanf_wrapper
 *
 * 
 *  @param integers[16]  Array of integer read
 *  @param characters[16]  Array of char read
 *  @param *channels  The pointer that will store the value of channels from read header
 *  @param *sampling_rate  The pointer that will store the value of channels from read header
 * 
 *  @return 1 if okay, else 0
 */
int second_read_extract(const int integers[16], const char characters[16], long *channels, long *sampling_rate)
{
    if (integers == NULL || characters == NULL || channels == NULL || sampling_rate == NULL)
    {
        return 0;
    }

    *channels = (integers[7] * 256) | integers[6];
    *sampling_rate = (integers[11] * 16777216) | (integers[10] * 65536) | (integers[9] * 256) | integers[8];
    return 1;
}

/*
 * This function will verify and extract information from third read of fscanf_wrapper
 *
 * 
 * @param integers[16]  Array of integer read
 * @param characters[16]  Array of char read
 * @param *data_size  The number of bytes in actual data of WAV file
 * @param *byte_per_sample  Byte per sample
 * 
 * @return 1 if okay, else 0
 */
int third_read_verify_and_extract(const int integers[16], const char characters[16], long *data_size, long *byte_per_sample)
{
    if (integers == NULL || characters == NULL || data_size == NULL || byte_per_sample == NULL)
    {
        return 0;
    }

    if (characters[4] != 'd' || characters[5] != 'a' || characters[6] != 't' || characters[7] != 'a')
    {
        return 0;
    }

    *data_size = (integers[11] * 16777216) | (integers[10] * 65536) | (integers[9] * 256) | integers[8];
    *byte_per_sample = (integers[1] * 256) | integers[0];
    return 1;
}

/*
 * This function will apply fade-in effect to the WAV file
 *
 * 
 * @param *samples  The array of samples bytes
 * @param num_samples  Number of bytes
 * @param sampling_rate  The sampling rate of WAV file
 * @param channels  The number of channels of the WAV file
 * @param milliseconds  The number of milliseconds to apply the effect
 * 
 */
void fade_in(short *samples, long num_samples, long sampling_rate, long channels, long milliseconds)
{
   
    long i;
    long num_samples_to_effect;
    long samples_processed;

    if (milliseconds == 0)
    {
        return;
    }

    samples_processed = 0;
    num_samples_to_effect = (long)(((double)sampling_rate / 1000.0 * (double)milliseconds));
    if (num_samples_to_effect > num_samples / channels)
    {
        num_samples_to_effect = num_samples / channels;
    }

    for (i = 0; i < num_samples_to_effect; i++)
    {
        if (channels == 1)
        {
            samples[samples_processed] = (short)(((double)samples[samples_processed] * (double)i) / (double)num_samples_to_effect);
            samples_processed++;
        }
        else
        {
            samples[samples_processed] = (short)(((double)samples[samples_processed] * (double)i) / (double)num_samples_to_effect);
            samples[samples_processed + 1] = (short)(((double)samples[samples_processed + 1] * (double)i) / (double)num_samples_to_effect);
            samples_processed += 2;
        }
    }
}

/*
 * This function will apply fade-out effect to the WAV file
 *
 * 
 * @param *samples  The array of samples bytes
 * @param num_samples  Number of bytes
 * @param sampling_rate  The sampling rate of WAV file
 * @param channels  The number of channels of the WAV file
 * @param milliseconds  The number of milliseconds to apply the effect
 *
 *
 */
void fade_out(short *samples, long num_samples, long sampling_rate, long channels, long milliseconds)
{
    
    long i;
    long num_samples_to_effect;
    long samples_processed;

    if (milliseconds == 0)
    {
        return;
    }

    num_samples_to_effect = (long)(((double)sampling_rate / 1000.0 * (double)milliseconds));
    if (num_samples_to_effect > num_samples / channels)
    {
        num_samples_to_effect = num_samples / channels;
    }

    samples_processed = (num_samples - (num_samples_to_effect * channels));
    for (i = num_samples - num_samples_to_effect; i < num_samples; i++)
    {
        if (channels == 1)
        {
            samples[samples_processed] = (short)(((double)samples[samples_processed] * (double)(num_samples - i - 1)) / (double)num_samples_to_effect);
            samples_processed++;
        }
        else
        {
            samples[samples_processed] = (short)(((double)samples[samples_processed] * (double)(num_samples - i - 1)) / (double)num_samples_to_effect);
            samples[samples_processed + 1] = (short)(((double)samples[samples_processed + 1] * (double)(num_samples - i - 1)) / (double)num_samples_to_effect);
            samples_processed += 2;
        }
    }
}

/*
 * This function will apply pan left-to-right effect to the WAV file
 *
 * 
 * @param *samples  The array of samples bytes
 * @param num_samples  Number of bytes
 * @param sampling_rate  The sampling rate of WAV file
 * @param channels  The number of channels of the WAV file
 * @param milliseconds  The number of milliseconds to apply the effect
 *
 */
void pan(short *samples, long num_samples, long sampling_rate, long channels, long milliseconds)
{
    
    long i;
    long num_samples_to_effect;
    long samples_processed;

    if (channels == 1)
    {
        return;
    }

    samples_processed = 0;
    num_samples_to_effect = (long)(((double)sampling_rate / 1000.0 * (double)milliseconds));
    if (num_samples_to_effect > num_samples / channels)
    {
        num_samples_to_effect = num_samples / channels;
    }

    for (i = 0; i < num_samples_to_effect; i++)
    {
        samples[samples_processed] = (short)(((double)samples[samples_processed] * (double)(num_samples_to_effect - i - 1)) / (double)num_samples_to_effect);
        samples[samples_processed + 1] = (short)(((double)samples[samples_processed + 1] * (double)i) / (double)num_samples_to_effect);
        samples_processed += 2;
    }

    for (i = num_samples_to_effect; i < num_samples / channels; i++)
    {
        samples[samples_processed] = 0;
        samples_processed += 2;
    }
}

int main(int argc, char **argv)
{
    
    unsigned line_number;// is used to read line number using fscanf_wrapper
    int integers[16];//The array that is used to read integers using fscanf_wrapper
    int integers_saved[16];//Array of saved integers from third read of fscanf_wrapper
    char characters[16];//The array this is used to read characters using fscanf_wrapper
    long milliseconds;//The number of milliseconds to put effect to
    int effect;//The effect, 0 = fade-in, 1 = fade-out, 2 = pan & -1 = error
    char *temporary;//To supply to strtol, a check argument
    int ret_fscanf;//The integer, that is returned by fscanf_wrapper
    int r;//The integer that store results of 'check' function above, and getopt
    long channels;//The channels of WAV file
    long sampling_rate;//The sampling rate of WAV file
    long data_size;//The size of 'data' of WAV file
    long effective_data_size;//There may be another frame at the end of WAV
    long byte_per_sample;//Bytes per sample, including all channels
    int i;
    unsigned char *samples;//The dynamically-allocated array to store samples
    long samples_written;//To keep track of how many samples were saved to 'samples'

    if (argc != 3)
    {
        printf("Error: Invalid command-line arguments.\n");
        return 1;
    }

    /*
     * Strtol tries to convert string (first argument) to long with base as third argument
     * If fails, it sets temporary (second argument) to first argument
     */
    milliseconds = strtol(argv[2], &temporary, 10);

    /*
     * If temporary which is supplied as third argument, is equal to argv[2],
     * which is first argument, conversion failed
     */
    if (temporary == argv[2])
    {
        printf("Error: Invalid command-line arguments.\n");
        return 1;
    }

    /*
     * Three possible effects in our list
     */
    if (strcmp(argv[1], "-fin") == 0)
    {
        effect = 0;
    }
    else if (strcmp(argv[1], "-fout") == 0)
    {
        effect = 1;
    }
    else if (strcmp(argv[1], "-pan") == 0)
    {
        effect = 2;
    }
    else
    {
        printf("Error: Invalid command-line arguments.\n");
        return 1;
    }

    ret_fscanf = fscanf_wrapper(stdin, &line_number, integers, characters);
    if (ret_fscanf != full_line_ret_fscanf)
    {
        printf("Error processing WAV file.\n");
        return 1;
    }
    r = first_read_check(integers, characters);
    if (r != 1)
    {
        printf("Error processing WAV file.\n");
        return 1;
    }

    /*
     * if ret_fscanf >= 16, it will read character array too, so, number of actual integer read are
     * ret_fscanf - 1, the one line number at start and then divided by 2, since for each integer, there is a character at end
     *
     * if ret_fscanf < 16, we only have line number and integer read, since scanf detects character but it is expecting integers,
     * it fails so, number of integer read are ret_fscanf - 1
     */
    if (ret_fscanf >=16){
        fprintf_wrapper(stdout, (ret_fscanf - 1) / 2, line_number, integers);
    }else{
        fprintf_wrapper(stdout, (ret_fscanf - 1), line_number, integers);
    }
    

    ret_fscanf = fscanf_wrapper(stdin, &line_number, integers, characters);
    if (ret_fscanf != full_line_ret_fscanf)
    {
        printf("Error processing WAV file.\n");
        return 1;
    }
    r = second_read_extract(integers, characters, &channels, &sampling_rate);
    if (r != 1)
    {
        printf("Error processing WAV file.\n");
        return 1;
    }

    
    if (ret_fscanf >=16){
        fprintf_wrapper(stdout, (ret_fscanf - 1) / 2, line_number, integers);
    }else{
        fprintf_wrapper(stdout, (ret_fscanf - 1), line_number, integers);
    }


    ret_fscanf = fscanf_wrapper(stdin, &line_number, integers, characters);
    if (ret_fscanf != full_line_ret_fscanf)
    {
        printf("Error processing WAV file.\n");
        return 1;
    }
    r = third_read_verify_and_extract(integers, characters, &data_size, &byte_per_sample);
    if (r != 1)
    {
        printf("Error processing WAV file.\n");
        return 1;
    }
    if (byte_per_sample != 4 && byte_per_sample != 2)
    {
        printf("Error processing WAV file.\n");
        return 1;
    }

    for (i = 0; i < 16; i++)
    {
        integers_saved[i] = integers[i];
    }

    samples = (unsigned char *)malloc((long)((double)data_size * 1.5) * sizeof(unsigned char));
    if (samples == NULL)
    {
        printf("Error: Malloc error.\n");
        return 1;
    }

    samples[0] = integers[12];
    samples[1] = integers[13];
    samples[2] = integers[14];
    samples[3] = integers[15];
    samples_written = 4;

    /*
     * Saving read samples to an array, it will make it easy to apply effect to it
     */
    effective_data_size = 4;
    while (1)
    {
        r = fscanf_wrapper(stdin, &line_number, integers, characters);
        if (r >= 16)
        {
            /*
             * Same principle as above, if fscanf gave r > 16, we have 16 integers full
             */
            for (i = 0; i < 16; i++)
            {
                samples[samples_written++] = integers[i];
                effective_data_size++;
            }
        }
        else
        {
            /*
             * Else we have integers one less than fscanf gave as first is line number
             */
            for (i = 0; i < r - 1; i++)
            {
                samples[samples_written++] = integers[i];
                effective_data_size++;
            }
            break;
        }
    }
    

    if (effect == 0)
    {
        fade_in((short *)samples, data_size / 2, sampling_rate, channels, milliseconds);
    }
    else if (effect == 1)
    {
        fade_out((short *)samples, data_size / 2, sampling_rate, channels, milliseconds);
    }
    else if (effect == 2)
    {
        pan((short *)samples, data_size / 2, sampling_rate, channels, milliseconds);
    }

    /*
     * Writing data, we will use 'samples_written' as control
     */
    line_number = 0x20;
    integers_saved[12] = samples[0];
    integers_saved[13] = samples[1];
    integers_saved[14] = samples[2];
    integers_saved[15] = samples[3];
    fprintf_wrapper(stdout, 16, line_number, integers_saved);

    samples_written = 4;
    while (samples_written < effective_data_size)
    {
        line_number += 0x10;
        /*
         * We are getting full line
         */
        if (effective_data_size - samples_written > 16)
        {
            /*
             * Saving all 16 integers and writing
             */
            for (i = 0; i < 16; i++)
            {
                integers_saved[i] = samples[samples_written+i];
            }
            fprintf_wrapper(stdout, 16, line_number, integers_saved);
            samples_written += 16;
        }
        else
        {
            /*
             * Saving what we have and writing
             */
            for (i = 0; i < data_size - samples_written; i++)
            {
                integers_saved[i] = samples[samples_written+i];
            }
            fprintf_wrapper(stdout, effective_data_size - samples_written, line_number, integers_saved);
            samples_written = effective_data_size;
        }
    }
    free(samples);
    return 0;
}

