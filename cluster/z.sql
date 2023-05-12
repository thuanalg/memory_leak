select 
agents.fixed_extension, 
monitor_recording.destination, 
queue_log.agent 
from agents, monitor_recording, queue_log 
where 
agents.agent_number=monitor_recording.destination 
and 
queue_log.callid=monitor_recording.uniqueid
and 
agents.agent_number <> agents.fixed_extension
and
INSTR(queue_log.agent, agents.fixed_extension) <= 0;
