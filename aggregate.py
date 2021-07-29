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
mem_trans    = '7 . number of memory transactions:'        
c2c_trans    = '8 . number of cache-to-cache transfers:'  
interven     = '9 . number of interventions:'              
invalid      = '10. number of invalidations:'              
flushes      = '11. number of flushes:'                    
bus_rds      = '12. number of SignalRds issued:'              
bus_rdx      = '13. number of SignalRdXs issued:'            
bus_upgrs    = '14. number of SignalUpgrs issued:'           
bus_wrs      = '15. number of BusWrs issued:'              

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
            elif mem_trans in line:
                output = output + ',' + line.replace(mem_trans,'').strip()
            elif c2c_trans in line:
                output = output + ',' + line.replace(c2c_trans,'').strip()
            elif interven in line:
                output = output + ',' + line.replace(interven,'').strip()
            elif invalid in line:
                output = output + ',' + line.replace(invalid,'').strip()
            elif flushes in line:
                output = output + ',' + line.replace(flushes,'').strip()
            elif bus_rds in line:
                output = output + ',' + line.replace(bus_rds,'').strip()
            elif bus_rdx in line:
                output = output + ',' + line.replace(bus_rdx,'').strip()
            elif bus_upgrs in line:
                output = output + ',' + line.replace(bus_upgrs,'').strip()
            elif bus_wrs in line:
                output = output + ',' + line.replace(bus_wrs,'').strip()       
        
        
        
        #Build csv headers
        csv_header = 'Cache_Number,'
        start_char = '.'
        end_char = ':'
        header_list = [reads,read_miss,writes,write_miss,miss_rate,writebacks,mem_trans,c2c_trans,interven,invalid,flushes,bus_rds,bus_rdx,bus_upgrs,bus_wrs]

        for col in header_list:
            csv_header = csv_header+col.split(start_char)[1].split(end_char)[0].replace('number of','').strip()+','

        output = csv_header+'\n'+output[1:]
        with open(f'{output_file}.csv','w') as out:
            out.write(output)
    




            
            


print('Done!')
