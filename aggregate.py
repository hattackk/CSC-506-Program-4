from os.path import isfile, join
from os import listdir


# Header format
l1_size      = 'L1_SIZE:'
l1_assoc     = 'L1_ASSOC:'
l1_blck      = 'L1_BLOCKSIZE:'
procs        = 'NUMBER OF PROCESSORS:'
protocol     = 'COHERENCE PROTOCOL:'
trace_file   = 'swaptions_truncated'
file_name    = 'ref.'

# Cache format
cache_header = '============ Simulation results (Cache '
reads        = '1 . number of reads:'                      
read_miss    = '2 . number of read misses:'                
writes       = '3 . number of writes:'                     
write_miss   = '4 . number of write misses:'               
miss_rate    = '5 . total miss rate:'                     
writebacks   = '6 . number of writebacks:'                        
c2c_trans    = '7 . number of cache-to-cache transfers:'
sig_rds      = '8 . number of SignalRds:'               
sig_rdx      = '9 . number of SignalRdXs:'               
sig_upg      = '10. number of SignalUpgrs'              
invalid      = '11. number of invalidations:'       
interven     = '12. number of interventions:'               

all_files = [f for f in listdir('./') if isfile(join('./', f)) and file_name in f]

for f in all_files:
    output_file_name = ''
    output = ''
    
    with open(f) as input_file:
        Lines = input_file.readlines()
        num_of_caches = 0
        current_cache = 0
        for line in Lines:
            if l1_size in line:
                output_file = line.replace(l1_size,'').strip()
            elif l1_assoc in line:
                output_file = output_file + '_' + line.replace(l1_assoc,'').strip()
            elif l1_blck in line:
                output_file = output_file + '_' + line.replace(l1_blck,'').strip()
            elif procs in line:
                num_of_caches = int(line.replace(procs, '').strip())
                output_file = output_file + '_' + str(num_of_caches)
            elif protocol in line:
                output_file = output_file + '_' + line.replace(protocol,'').strip()
            elif 'TRACE FILE' in line:
                continue
            elif cache_header in line:
                output = output +'\n'
                output = output +f'Cache_{current_cache},'
                current_cache = current_cache + 1
            elif reads in line:
                output = output + line.replace(reads,'').strip()
            elif read_miss in line:
                output = output + ',' + line.replace(read_miss,'').strip()
            elif writes in line:
                output = output + ',' + line.replace(writes,'').strip()
            elif write_miss in line:
                output = output + ',' + line.replace(write_miss,'').strip()
            elif miss_rate in line:
                output = output + ',' + line.replace(miss_rate,'').strip()
            elif writebacks in line:
                output = output + ',' + line.replace(writebacks,'').strip()
            elif c2c_trans in line:
                output = output + ',' + line.replace(c2c_trans,'').strip()
            elif sig_rds in line:
                output = output+ ',' + line.replace(sig_rds,'').strip()
            elif sig_rdx in line:
                output = output + ',' + line.replace(sig_rdx,'').strip()
            elif sig_upg in line:
                output = output + ',' + line.replace(sig_upg,'').strip()       
            elif invalid in line:
                output = output + ',' + line.replace(invalid,'').strip()
            elif interven in line:
                output = output + ',' + line.replace(interven,'').strip()
        
        
        
        #Build csv headers
        csv_header = 'Cache_Number,'
        start_char = '.'
        end_char = ':'
        header_list = [reads,read_miss,writes,write_miss,miss_rate,writebacks,c2c_trans,sig_rds,sig_rdx,sig_upg,invalid,interven]

        for col in header_list:
            csv_header = csv_header+col.split(start_char)[1].split(end_char)[0].replace('number of','').strip()+','

        output = csv_header+'\n'+output[1:]
        with open(f'{output_file}.csv','w') as out:
            out.write(output)
    




            
            


print('Done!')
