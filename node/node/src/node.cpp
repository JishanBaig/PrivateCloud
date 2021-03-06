/*
author = 'JBaig'

Node Controller-
* Node Controller manages virtual machines on a physical machine(node).
* All the Node controllers are controlled and managed by Cluster Controller.
*/

#include "node.h"
#include <stdlib.h>

using namespace std;

std::string exec(const char* cmd) {
    char buffer[128];
    std::string result = "";
    std::shared_ptr<FILE> pipe(popen(cmd, "r"), pclose);
    if (!pipe) throw std::runtime_error("popen() failed!");
    while (!feof(pipe.get())) {
        if (fgets(buffer, 128, pipe.get()) != NULL)
            result += buffer;
    }
    return result;
}

# Db Class to instantiate node with initial parameters.
class Db{
public:
    static Db& inst()
    {
        static Db instance;
        if (instance.init != true) {
           cout<<"Node Controller is starting..."<<'\n';   
           virInitialize();
           //unordered_map<string,virDomainInfo> allVMinfo;
          
           cout<<"connection creation"<<endl;
           instance.connection = virConnectOpen("qemu:///system");
           if (instance.connection == NULL) {
             fprintf(stderr, "Failed to open connection to qemu:///system\n");
           }
           cout<<"I received a message from Cluster Contoller/Client so let me interpret it."<<'\n';
           instance.numofVMs=0;
           int vcpus = virConnectGetMaxVcpus(instance.connection, NULL);
           fprintf(stdout, "Maximum support virtual CPUs: %d\n", vcpus);
           unsigned long long node_free_memory = virNodeGetFreeMemory(instance.connection);
           fprintf(stdout, "Node free memory: %llu\n", node_free_memory);
           instance.myinfo.availablememory=node_free_memory;
           instance.myinfo.availablevcpus=vcpus;
           instance.myinfo_busy.availablememory=0;
           instance.myinfo_busy.availablevcpus=0;
           instance.myinfo_all.availablememory=node_free_memory;
           instance.myinfo_all.availablevcpus=vcpus;
           instance.init = true;
           instance.firstoneflag=0;
           instance.first_dom = NULL;
          }
          return instance;
    }
    virConnectPtr connection;
    int numofVMs;
    hostinformation myinfo, myinfo_busy, myinfo_all;
    unordered_map<string,virDomainInfo> allVMinfo;
    int firstoneflag;
    virDomainPtr first_dom;
    //unordered_map<string,virDomainInfo>::iterator allVMinfoit;
private:
   
    bool init;
    
        };


//Create Virtual Machine with the name passes by Cluster Controller.
string node::createVM(const std::string& text)
{
  
  ifstream inFile;
  inFile.open("dom.txt");//open the input file
  virNodeInfo nodeinfo;
  stringstream strStream;
  strStream << inFile.rdbuf();//read the file
  string str =strStream.str();//str holds the content of the file
  string myvmname(text);

    int k=str.find("<name>");
    string s="<name>";
    //
    //int suffix=Db::inst().numofVMs;
    Db::inst().numofVMs++;
    //ostringstream ss;
    //ss << suffix;
    //string tobereplaced="centos"+ss.str();
    int l=s.length();
    k=k+l; //reached at the start of where i want to replace. 
    int m=str.find("</name>");
    cout<<k<<m<<endl;
    str.replace(k,m-k,myvmname);
    
    //
    cout<<"Domain creation from xml named"<<myvmname<<endl;
    virDomainPtr dom = virDomainCreateXML(Db::inst().connection,str.c_str(),0);
    //virDomainPtr dom=virDomainLookupByName(connection,"centos10");
     if(Db::inst().firstoneflag == 0)
       { Db::inst().first_dom = dom;     Db::inst().firstoneflag++; } 
    virDomainInfoPtr info=(virDomainInfoPtr)malloc(sizeof(virDomainInfo));
    if(virDomainGetInfo(dom, info)==-1)
    cout<<"Unable to get domain info";
    virDomainInfo VMinfo;
    VMinfo.state=info->state;
    VMinfo.maxMem= info->maxMem;
    VMinfo.memory= info->memory;
    VMinfo.nrVirtCpu= info->nrVirtCpu;
    VMinfo.cpuTime= info->cpuTime;
    cout<<"Hi, i am newly created VM, "<< myvmname <<"My allocated memory is "<<VMinfo.memory<<", and my allocated number of vcpus is "<<VMinfo.nrVirtCpu<<".\n";

    Db::inst().allVMinfo[myvmname]=VMinfo;

    cout<<"allVMinfo size : "<<Db::inst().allVMinfo.size()<<'\n';

    int vcpus = virConnectGetMaxVcpus(Db::inst().connection, NULL);
    fprintf(stdout, "Maximum support virtual CPUs: %d\n", vcpus);
    unsigned long long node_free_memory = virNodeGetFreeMemory(Db::inst().connection);
    fprintf(stdout, "Node free memory: %llu\n", node_free_memory);
    virNodeGetInfo(Db::inst().connection, &nodeinfo);
    //updating remaining resources info.
    Db::inst().myinfo.availablememory=node_free_memory;
    Db::inst().myinfo.availablevcpus=Db::inst().myinfo.availablevcpus - VMinfo.nrVirtCpu;
    Db::inst().myinfo_busy.availablememory = Db::inst().myinfo_busy.availablememory + VMinfo.memory;
    Db::inst().myinfo_busy.availablevcpus=Db::inst().myinfo_busy.availablevcpus + VMinfo.nrVirtCpu;
  return "VM created as you asked me by name";  
} 









//Create Virtual Machines with the given parameters passes by Cluster Controller.
//numVMs = number of virtual machines
//vcpu = number of cpus
//memory = memory size
string node::createVMreq(int numVMs,int vcpu,int memory)
{
  //string conf_file = ; 
  //FILE* f = open(conf_file); 
  //xml = f.read() 
  //conn = libvirt.open('xen:///') 
  //conn.defineXML(xml)
  //getDomainInfo(0);
  
  ifstream inFile;
  inFile.open("dom.txt");//open the input file
  virNodeInfo nodeinfo;
  stringstream strStream;
  strStream << inFile.rdbuf();//read the file
  string str =strStream.str();//str holds the content of the file
  //string str(textinfo);
  //textinfo.copy(str);
  //cout << str << endl;
  //cout<<str.length()<<endl;
  //cout<<str<<endl;
  while(numVMs--){
    int k=str.find("<name>");
    string s="<name>";
    //
    int suffix=Db::inst().numofVMs;
    Db::inst().numofVMs++;
    ostringstream ss;
    ss << suffix;
    string tobereplaced="centos"+ss.str();
    int l=s.length();
    k=k+l; //reached at the start of where i want to replace. 
    int m=str.find("</name>");
    cout<<k<<m<<endl;
    str.replace(k,m-k,tobereplaced);
    //
    ostringstream sp;
    sp << vcpu;
    string tobereplacedforvcpu=sp.str();
    ostringstream su;
    su << memory;
    string tobereplacedformemory=su.str();
    //
    k=str.find("<vcpu>");
    s="<vcpu>";
    l=s.length();
    k=k+l; //reached at the start of where i want to replace. 
    m=str.find("</vcpu>");
    cout<<k<<m<<endl;
    str.replace(k,m-k,tobereplacedforvcpu);
    //
    k=str.find("<memory>");
    s="<memory>";
    l=s.length();
    k=k+l; //reached at the start of where i want to replace. 
    m=str.find("</memory>");
    cout<<k<<m<<endl;
    str.replace(k,m-k,tobereplacedformemory);
    //
    cout<<"Domain creation from xml named"<<tobereplaced<<endl;
    virDomainPtr dom=virDomainCreateXML(Db::inst().connection,str.c_str(),0);
    //virDomainPtr dom=virDomainLookupByName(connection,"centos10");
    if(Db::inst().firstoneflag == 0)
       { Db::inst().first_dom = dom;     Db::inst().firstoneflag++; }
    virDomainInfoPtr info=(virDomainInfoPtr)malloc(sizeof(virDomainInfo));
    if(virDomainGetInfo(dom, info)==-1)
    cout<<"Unable to get domain info";
    virDomainInfo VMinfo;
    VMinfo.state=info->state;
    VMinfo.maxMem= info->maxMem;
    VMinfo.memory= info->memory;
    VMinfo.nrVirtCpu= info->nrVirtCpu;
    VMinfo.cpuTime= info->cpuTime;
    cout<<"Hi, i am newly created VM, "<< tobereplaced <<"My allocated memory is "<<VMinfo.memory<<", and my allocated number of vcpus is "<<VMinfo.nrVirtCpu<<".\n";

    Db::inst().allVMinfo[tobereplaced]=VMinfo;

    cout<<"allVMinfo size : "<<Db::inst().allVMinfo.size()<<'\n';



    //cout<<"listing active domain ids after creations"<<endl;
    //listingdomains(connection);
    //virDomainCreate(dom);
    //listingdomains(connection);
    //string in;
    //virDomainInfoPtr info;
    //cin>>in;
    //cout<<<<endl;
    //virDomainGetInfo(dom,info);
    //cout<<"domain state after creation : "<<info->state<<endl;
    //fprintf(stdout, "Virtualization type: %s\n", virConnectGetType(Db::inst().connection));
    //unsigned long ver;
    //virConnectGetVersion(Db::inst().connection, &ver);
    //fprintf(stdout, "Version: %lu\n", ver);
    //char * uri = virConnectGetURI(Db::inst().connection);
    //fprintf(stdout, "Canonical URI: %s\n", uri);
    //free(uri);
    //char *host = virConnectGetHostname(Db::inst().connection);
    //fprintf(stdout, "Hostname:%s\n", host);
    int vcpus = virConnectGetMaxVcpus(Db::inst().connection, NULL);
    fprintf(stdout, "Maximum support virtual CPUs: %d\n", vcpus);
    unsigned long long node_free_memory = virNodeGetFreeMemory(Db::inst().connection);
    fprintf(stdout, "Node free memory: %llu\n", node_free_memory);
    virNodeGetInfo(Db::inst().connection, &nodeinfo);
    //fprintf(stdout, "Model: %s\n", nodeinfo.model);
    //fprintf(stdout, "Memory size: %lukb\n", nodeinfo.memory);
    //fprintf(stdout, "Number of CPUs: %u\n", nodeinfo.cpus);
    //fprintf(stdout, "MHz of CPUs: %u\n", nodeinfo.mhz);
    //fprintf(stdout, "Number of NUMA nodes: %u\n", nodeinfo.nodes);
    //fprintf(stdout, "Number of CPU sockets: %u\n", nodeinfo.sockets);
    //fprintf(stdout, "Number of CPU cores per socket: %u\n", nodeinfo.cores);
    //fprintf(stdout, "Number of CPU threads per core: %u\n", nodeinfo.threads);
 
    //free(host);
    //updating remaining resources info.
    Db::inst().myinfo.availablememory=node_free_memory;
    Db::inst().myinfo.availablevcpus=Db::inst().myinfo.availablevcpus - VMinfo.nrVirtCpu;
    Db::inst().myinfo_busy.availablememory = Db::inst().myinfo_busy.availablememory + VMinfo.memory;
    Db::inst().myinfo_busy.availablevcpus=Db::inst().myinfo_busy.availablevcpus + VMinfo.nrVirtCpu;
  }
  return "VM created as you asked me by name";  
} 


//Suspend Virtual Machine with the given name passed by Cluster Controller.
string node::suspendVM(const std::string& text)
{
  cout<<"Domain suspended"<<endl;
  const char * domname = text.c_str();
  virDomainPtr dom=virDomainLookupByName(Db::inst().connection,domname);
  virDomainSuspend(dom);
  //const char* 
  string name = virDomainGetName(dom);
  Db::inst().allVMinfo[name].state= VIR_DOMAIN_PMSUSPENDED;
  //cout<<info->state<<endl; 
  //cout<<"allVMinfo size : "<<allVMinfo.size()<<'\n';  
  //cout<<"listing active domain ids after distructions"<<endl;
  //listingdomains(connection);
  //listinginactivedomains(connection);
  //virConnectClose(connection);
  return "VM suspended as you asked me by sending me "+ text;
}  


// Resume Virtual Machine with the given name passed by Cluster Controller.
string node::resumeVM(const std::string& text)
{
  cout<<"Domain resumed"<<endl;
  const char * domname = text.c_str();
  virDomainPtr dom=virDomainLookupByName(Db::inst().connection,domname);
  virDomainResume(dom);
  //const char* 
  string name = virDomainGetName(dom);
  Db::inst().allVMinfo[name].state= VIR_DOMAIN_RUNNING;
  //cout<<info->state<<endl; 
  //cout<<"allVMinfo size : "<<allVMinfo.size()<<'\n';  
  //cout<<"listing active domain ids after distructions"<<endl;
  //listingdomains(connection);
  //listinginactivedomains(connection);
  //virConnectClose(connection);
  return "VM resumed as you asked me by sending me "+ text;
}  







// Destroy Virtual Machine with the given name passed by Cluster Controller.
string node::destroyVM(const std::string& text)//const std::string& text)
{
  cout<<"Domain destruction"<<endl;
  const char * domname = text.c_str();
  virDomainPtr dom=virDomainLookupByName(Db::inst().connection,domname);
  virDomainDestroy(dom);
  //const char* 
  string name = virDomainGetName(dom);
  //Db::inst().allVMinfo.erase(name);
  //cout<<info->state<<endl; 
  cout<<"allVMinfo size : "<<Db::inst().allVMinfo.size()<<'\n';  
  cout<<"listing active domain ids after destructions"<<endl;
  //listingdomains(connection);
  //listinginactivedomains(connection);
  unsigned long long node_free_memory = virNodeGetFreeMemory(Db::inst().connection);
  fprintf(stdout, "Node free memory: %llu\n", node_free_memory);
   
  Db::inst().myinfo.availablememory = node_free_memory;
  Db::inst().myinfo.availablevcpus=Db::inst().myinfo.availablevcpus + Db::inst().allVMinfo[name].nrVirtCpu;
  Db::inst().myinfo_busy.availablememory = Db::inst().myinfo_busy.availablememory - Db::inst().allVMinfo[name].memory;
  Db::inst().myinfo_busy.availablevcpus=Db::inst().myinfo_busy.availablevcpus - Db::inst().allVMinfo[name].nrVirtCpu;

  Db::inst().allVMinfo.erase(name);

  virConnectClose(Db::inst().connection);
  return "VM destroyed as you asked me by sending me "+ text;
}  


// Destroy Virtual Machine by Cloud Controller as notified by user.
string node::destroyVMdir()//const std::string& text)
{
  cout<<"Domain destruction"<<endl;
  int num_destroyed_dom=0;
  unordered_map<string,virDomainInfo>::iterator allVMinfoit;
  for(allVMinfoit = Db::inst().allVMinfo.begin(); allVMinfoit != Db::inst().allVMinfo.end(); allVMinfoit++ ){
    virDomainPtr destroying_domain=virDomainLookupByName(Db::inst().connection, allVMinfoit->first.c_str());
    if( destroying_domain == Db::inst().first_dom )  continue;
    if(num_destroyed_dom == 1) break;
    virDomainDestroy(destroying_domain); 
    string name = virDomainGetName(destroying_domain);
    //Db::inst().allVMinfo.erase(name);
    //cout<<info->state<<endl; 
    cout<<"allVMinfo size : "<<Db::inst().allVMinfo.size()<<'\n';  
    cout<<"listing active domain ids after destructions"<<endl;
    //listingdomains(connection);
    //listinginactivedomains(connection);
    unsigned long long node_free_memory = virNodeGetFreeMemory(Db::inst().connection);
    fprintf(stdout, "Node free memory: %llu\n", node_free_memory);
    Db::inst().myinfo.availablememory = node_free_memory;
    Db::inst().myinfo.availablevcpus=Db::inst().myinfo.availablevcpus + Db::inst().allVMinfo[name].nrVirtCpu;
    Db::inst().myinfo_busy.availablememory = Db::inst().myinfo_busy.availablememory - Db::inst().allVMinfo[name].memory;
    Db::inst().myinfo_busy.availablevcpus=Db::inst().myinfo_busy.availablevcpus - Db::inst().allVMinfo[name].nrVirtCpu;
    Db::inst().allVMinfo.erase(name);
    num_destroyed_dom++;
  }
  //const char * domname = text.c_str();
  //virDomainPtr dom=virDomainLookupByName(Db::inst().connection,domname);
  virConnectClose(Db::inst().connection);
  return "VM destroyed directly as you asked me by sending me\n";
}  



//Returns Available Resources to Cluster Controller.
hostinformation node::getavailResources(){
cout<<"my available cpus : "<<Db::inst().myinfo.availablevcpus<<'\n';
cout<<"my available memory : "<<Db::inst().myinfo.availablememory<<'\n';
return Db::inst().myinfo;  
}


//Returns Busy Resources to Cluster Controller.
hostinformation node::getbusyResources(){
cout<<"my busy cpus : "<<Db::inst().myinfo_busy.availablevcpus<<'\n';
cout<<"my busy memory : "<<Db::inst().myinfo_busy.availablememory<<'\n';
return Db::inst().myinfo_busy;  
}

//Returns All the Resources to the Cluster Controller.
hostinformation node::getResources(){
return Db::inst().myinfo_all;  
}


// Migrate Virtual Machine with the given name passed by Cluster Controller.
string node::migrateVMs(const std::string& text)
{
  cout<<"Domain migration starts."<<'\n';
  string p(text);
  const char * target_uri = p.c_str();
  int numVMs_migrated = 0;
  unordered_map<string,virDomainInfo>::iterator allVMinfoit;
  for(allVMinfoit = Db::inst().allVMinfo.begin(); allVMinfoit != Db::inst().allVMinfo.end(); allVMinfoit++ ){
  	cout<<"migrating domain name = "<< allVMinfoit->first <<'\n';
    virDomainPtr migrating_domain=virDomainLookupByName(Db::inst().connection, allVMinfoit->first.c_str());
    ostringstream q;
    q << numVMs_migrated;
    string r = "migrated_centos" + q.str();
    const char* name_at_target =r.c_str();
    string tu(target_uri);
    int x = tu.find(':');
    string s = tu.substr(0,x);

    stringstream ss; 
    ss << "qemu+ssh://" << s << "/system";
    string conn = ss.str();
    cout<<"conn = "<<conn<<'\n';
    virConnectPtr connect = virConnectOpen(conn.c_str()); 
    cout<<" i am migrating.\n";
    string ur = "tcp://" + s;
    cout<< "target uri" << ur <<'\n';

    virDomainPtr ptr = virDomainMigrate(migrating_domain, connect, VIR_MIGRATE_LIVE , name_at_target, ur.c_str() , 1);
    //int success = virDomainMigrateToURI(migrating_domain, target_uri,  VIR_MIGRATE_LIVE , name_at_target, 1);
    if(ptr == NULL){
    	cout<<"Migration failed\n";
    	return "Migration failed.";
    }
    //Db::inst().allVMinfo.erase(allVMinfoit->first);
    cout<<"one vm done.\n";
    numVMs_migrated++;
    //sleep(2000);
  }


  // virDomainPtr	virDomainMigrate	(virDomainPtr domain,
					// virConnectPtr dconn,
					 //unsigned long flags,
					 //const char * dname,
					 //const char * uri,
					 //unsigned long bandwidth)
  //const char * domname = text.c_str();
  //virDomainPtr dom=virDomainLookupByName(Db::inst().connection,domname);
  //virDomainDestroy(dom);
  //const char* 
  //string name = virDomainGetName(dom);
  //Db::inst().allVMinfo.erase(name);
  //cout<<info->state<<endl; 
  //cout<<"allVMinfo size : "<<Db::inst().allVMinfo.size()<<'\n';  
  //cout<<"listing active domain ids after destructions"<<endl;
  //listingdomains(connection);
  //listinginactivedomains(connection);
  //unsigned long long node_free_memory = virNodeGetFreeMemory(Db::inst().connection);
  //fprintf(stdout, "Node free memory: %llu\n", node_free_memory);
   

  //Db::inst().myinfo.availablememory = node_free_memory;
  //Db::inst().myinfo.availablevcpus=Db::inst().myinfo.availablevcpus + Db::inst().allVMinfo[name].nrVirtCpu;
  //Db::inst().myinfo_busy.availablememory = Db::inst().myinfo_busy.availablememory - Db::inst().allVMinfo[name].memory;
  //Db::inst().myinfo_busy.availablevcpus=Db::inst().myinfo_busy.availablevcpus - Db::inst().allVMinfo[name].nrVirtCpu;

  //Db::inst().allVMinfo.erase(name);

  //virConnectClose(Db::inst().connection);
  return "all the VMs has been migrated to " + text + " as you asked me by sending the request.\n";
}  







//create volume.
std::string node::createVolume(std::string domain, std::string cap, std::string unit) {
  string tag = unit;
  long long capacity = atoll(cap.c_str());
   if(tag != "K" && tag != "M" && tag != "G" && tag != "T"){

      fprintf(stderr, "capacity should be in K, M, G or T\n");
      return " volume creation failed.\n";

  }
    virConnectPtr conn;
    virStoragePoolPtr *ptr = NULL;
    virStoragePoolPtr poolPtr;  //image pool ptr
    virStoragePoolInfo info; //info related to pool
    bool canAllocate = false;

   stringstream ns;
   
   string node="";

   if(node == ""){
      ns << "qemu:///system";
   }else{
      ns << "qemu+ssh://" << node << "/system";
   }
    string connectionstring = ns.str();

    conn = virConnectOpen(connectionstring.c_str());

    if (conn == NULL) {
        fprintf(stderr, "Failed to open connection to %s\n", connectionstring.c_str());
        return "failed to open connection";
    }else{

      fprintf(stdout, "Connected to %s\n", connectionstring.c_str());
    }

    int no_pools = virConnectListAllStoragePools(conn, &ptr, VIR_CONNECT_LIST_STORAGE_POOLS_ACTIVE);

    fprintf(stdout, "No of Storage pools are %d\n", no_pools);

    //cout << virStoragePoolGetName(*ptr) << endl;

    for( int i = 1 ; i <= no_pools ; ++i, ptr++){
      poolPtr = *ptr;
      string poolName = string(virStoragePoolGetName(poolPtr));
       cout << poolName << endl;
            

    if(virStoragePoolGetInfo( poolPtr,&info) == 0){  //If success
    

      fprintf(stdout, "State: %d\n", info.state);
      fprintf(stdout, "Capacity: %lld G\n", info.capacity/(1024*1024*1024));
      fprintf(stdout, "Allocation: %lld G\n", info.allocation/(1024*1024*1024));
      fprintf(stdout, "Available: %lld G\n", info.available/(1024*1024*1024));
      
      long long dom_capacity = info.capacity;
      

      if(tag == "K"){
        dom_capacity = dom_capacity / K;
      }else if(tag == "M"){
        dom_capacity = dom_capacity / M;
      }else if(tag == "G"){
        dom_capacity = dom_capacity / G;
      }else if(tag == "T"){
        dom_capacity = dom_capacity / T;
      }

      if(dom_capacity <= 0 || (dom_capacity - capacity) < 0){   //If the node can't afford a capacity specified by the user return false (-1)
        
        canAllocate = false;
        return "node cant afford a capacity specified by the user.";
      }else{
        canAllocate = true;
        break;
      }

    }
    
      virStoragePoolFree(poolPtr);
    } //for loop


  //We reached here because there were errors reading the info of the Pools

  if(canAllocate = false){
    //TODO: We can create a pool named images if it's not present
    fprintf(stderr, "Pool unreadable exception\n"); //Return false if image Pool is not present
    return "Pool unreadable exception.";
  }

  //all's well..allocate the specified space.
  int no_of_volumes =   virStoragePoolNumOfVolumes(poolPtr);  //Get the number of volumes inside the pool images

  fprintf(stdout, "\nNo of volumes : %d\n", no_of_volumes);


  //create XML for Volume 
  stringstream ss;
  ss << "<volume type=\'file\'><name>";
  ss << "Volume" << no_of_volumes + 1;
  ss << "</name><key>/var/lib/virt/images/Volume" << no_of_volumes + 1 << ".qcow2</key><source></source><capacity unit=\"";
  ss << tag << "\">" << capacity << "</capacity><allocation unit=\"" +tag+"\">0</allocation><target><path>/var/lib/virt/images/Volume" << no_of_volumes + 1 << ".qcow2</path>";
  ss << "<format type=\'qcow2\'/><permissions><owner>107</owner><group>107</group><mode>0744</mode>";
  ss << "</permissions><compat>1.1</compat><features><lazy_refcounts/></features></target></volume>";

  //XML created for Volume
  string volXML = ss.str();
  cout << volXML<< endl;
  
  //create a volume out of XML
  virStorageVolPtr volPtr = virStorageVolCreateXML(poolPtr, volXML.c_str(), 1);

  if(volPtr == NULL){  //Error in creation of the volume
    fprintf(stdout, "\nCould't create volume\n");
    return "couldn't create volume.";
  }

  //Create XML for Disk
  stringstream ds;
  ds << "<disk type=\'file\' device=\'disk\'><driver name=\'qemu\' type=\'qcow2\'/><source file=\'/var/lib/libvirt/images/";
  ds << "Volume" << no_of_volumes + 1 << "\' /><target dev=\'sda\' bus=\'usb\'/></disk> ";


  string diskXML =  ds.str();   

  cout << "\nDisk XML :\n" << diskXML << endl;
  //Now attach the storage to the VM pointed by domain

  virDomainPtr *domains;
  bool domainFound = false;
  size_t i;
  int ret;
  unsigned int flags = VIR_CONNECT_LIST_DOMAINS_SHUTOFF | VIR_CONNECT_LIST_DOMAINS_RUNNING | VIR_CONNECT_LIST_DOMAINS_PAUSED;
  

  ret = virConnectListAllDomains(conn, &domains, flags);
  
  cout << "Domain count: " << ret << endl;

  if (ret < 0){   //Error
    fprintf(stdout, "Can't find all the domains\n");
    return "cant find all the domains.";
  }
  for (i = 0; i < ret; i++) {
       string domainName = string(virDomainGetName(domains[i]));
       cout << "Domain Name : " << domainName << endl;

       if(domainName == domain){
        //we have found our domain  
        domainFound = true;

        if(virDomainAttachDevice(domains[i],diskXML.c_str()) == 0){
          fprintf(stdout, "Storage Volume attached to the domain as USB Device.\n");
        }

        break;
       }
       //here or in a separate loop if needed
       virDomainFree(domains[i]);
  }

    virConnectClose(conn);
    return "create volume done";
}




//get pool list.
std::vector<std::string> node::getPoolList() {
  vector<string> poolList;
  string node="";
  virConnectPtr conn;
    virStoragePoolPtr *ptr = NULL;
    
    stringstream ss;
     
   if(node == ""){
      ss << "qemu:///system";
   }else{
      ss << "qemu+ssh://" << node << "/system";
   }

    string connectionstring = ss.str();

    conn = virConnectOpen(connectionstring.c_str());

    if (conn == NULL) {
        fprintf(stderr, "Failed to open connection to %s\n", connectionstring.c_str());
        return poolList;
    }else{

      fprintf(stdout, "Connected to %s\n", connectionstring.c_str());
    }

    int no_pools = virConnectListAllStoragePools(conn, &ptr, VIR_CONNECT_LIST_STORAGE_POOLS_ACTIVE);

    fprintf(stdout, "No of Storage pools are %d\n", no_pools);

    for(int i = 0; i < no_pools ; ++i){
      poolList.push_back(string(virStoragePoolGetName(ptr[i])));
      virStoragePoolFree(ptr[i]);
    }


     virConnectClose(conn);


    return poolList;

}



//get volume list.
std::vector<std::string> node::getVolumeList(std::string pool) {
  vector<string> volList;
  string node="";
  virConnectPtr conn;
    virStoragePoolPtr *ptr = NULL;
    stringstream ss;
     
   if(node == ""){
      ss << "qemu:///system";
   }else{
      ss << "qemu+ssh://" << node << "/system";
   }


    string connectionstring = ss.str();

    conn = virConnectOpen(connectionstring.c_str());

    if (conn == NULL) {
        fprintf(stderr, "Failed to open connection to %s\n", connectionstring.c_str());
        return volList;
    }else{

      fprintf(stdout, "Connected to %s\n", connectionstring.c_str());
    }

    int no_pools = virConnectListAllStoragePools(conn, &ptr, VIR_CONNECT_LIST_STORAGE_POOLS_ACTIVE);

    fprintf(stdout, "No of Storage pools are %d\n", no_pools);

    for(int i = 0; i < no_pools ; ++i){
      string poolName = string(virStoragePoolGetName(ptr[i]));

      if(poolName == pool){
        int no_Vol = virStoragePoolNumOfVolumes(ptr[i]);
        fprintf(stdout , "\nNumber of Volumes are %d\n", no_Vol);
        
        virStorageVolPtr *volPtr;

        virStoragePoolListAllVolumes(ptr[i], &volPtr, 0);

        if(volPtr != NULL){
          //List all the volumes one by one;

          for(int j = 0 ; j < no_Vol ; ++j){
            volList.push_back(string(virStorageVolGetName(volPtr[j])));
          }
          
        }
        break;
      }
      virStoragePoolFree(ptr[i]);
    }

    virConnectClose(conn);
    return volList;
}



//attach volume to virtual machine. 
string node::attachVolumeToVM(std::string domain, std::string pool, std::string volume) {
    virConnectPtr conn;
    virStoragePoolPtr *ptr = NULL;
    string node = "";
    virStorageVolPtr reqPtr;


    stringstream ss;
     
   if(node == ""){
      ss << "qemu:///system";
   }else{
      ss << "qemu+ssh://" << node << "/system";
   }

    string connectionstring = ss.str();

    conn = virConnectOpen(connectionstring.c_str());

    if (conn == NULL) {
        fprintf(stderr, "Failed to open connection to %s\n", connectionstring.c_str());
        return "failed to open connection";
    }else{

      fprintf(stdout, "Connected to %s\n", connectionstring.c_str());
    }

    int no_pools = virConnectListAllStoragePools(conn, &ptr, VIR_CONNECT_LIST_STORAGE_POOLS_ACTIVE);

    fprintf(stdout, "No of Storage pools are %d\n", no_pools);

    for(int i = 0; i < no_pools ; ++i){
      string poolName = string(virStoragePoolGetName(ptr[i]));

      if(poolName == pool){
        int no_Vol = virStoragePoolNumOfVolumes(ptr[i]);
        fprintf(stdout , "\nNumber of Volumes are %d\n", no_Vol);
        
        virStorageVolPtr *volPtr;

        virStoragePoolListAllVolumes(ptr[i], &volPtr, 0);

        if(volPtr != NULL){
          //List all the volumes one by one;

          for(int j = 0 ; j < no_Vol ; ++j){
            if(string(virStorageVolGetName(volPtr[j])) == volume){
              reqPtr = volPtr[j];
              break;
            }
            virStorageVolFree(volPtr[j]);
          }

        }else{
          return "return -1";
        }
        break;
      }
      virStoragePoolFree(ptr[i]);
    }

    //Get the path of the Volume selected
    string path = string(virStorageVolGetPath(reqPtr));

   
    //Create XML for Disk
  stringstream ds;

  ds << "<disk type=\'file\' device=\'disk\'><driver name=\'qemu\' type=\'qcow2\'/><source file=\'";
  ds << path << "\'/><target dev=\'sda\' bus=\'usb\'/></disk> ";

  string diskXML = ds.str();

  virDomainPtr *domains;
  
  size_t i;
  int ret;
  unsigned int flags = VIR_CONNECT_LIST_DOMAINS_SHUTOFF | VIR_CONNECT_LIST_DOMAINS_RUNNING | VIR_CONNECT_LIST_DOMAINS_PAUSED;
  

  ret = virConnectListAllDomains(conn, &domains, flags);
  
  cout << "Domain count: " << ret << endl;

  if (ret < 0){   //Error
    fprintf(stdout, "Can't find all the domains\n");
    return "cant find all the domains.";
  }
  for (i = 0; i < ret; i++) {
       string domainName = string(virDomainGetName(domains[i]));
       cout << "Domain Name : " << domainName << endl;

       if(domainName == domain){
        //we have found our domain  
        

        if(virDomainAttachDevice(domains[i],diskXML.c_str()) == 0){
          fprintf(stdout, "Storage Volume attached to the domain as USB Device.\n");
        }

        break;
       }
       //here or in a separate loop if needed
       virDomainFree(domains[i]);
  }

    virConnectClose(conn);

    return "attach volume to vm done.";
}

//upload object
std::string node::objectUpload(std::string fileName, std::string content) {
  	string node="";
  	virConnectPtr conn;
    stringstream ss;
    ofstream fs;
	string cmd;
	string result;
	 string name= fileName + ".txt";
	fs.open(name.c_str());
	fs << content;
	fs.close();
	
	//check if bucket exists?
	result = exec("ls | grep BlockStorage.img");
	cout << "Result of ls command..\n" << result << endl;
	if(result == ""){
		cout << "BlockStorage.img is not present..." << endl;
		cout << "Creating the BlcokStore" << endl;
		cmd = "dd if=/dev/zero of=BlockStorage.img bs=1M count=1000";

		system(cmd.c_str());

		cout << "\nCreated a BlockStore" << endl;
		cout << "\nFormatting it to fat fs" << endl;
		cmd = "mkfs fat -F BlockStorage.img";
		system(cmd.c_str());
		cout << "\nFormatting complete" << endl;
		cout << "\nCreating mount point" << endl;
		cmd = "mkdir mountPoint";
		system(cmd.c_str());
		cmd = "chmod -R ug+rw mountPoint";

		system(cmd.c_str());

		cout << "Done creating disk and mount points" << endl; 
	}

	//Now that we are done creating disks and mount points, we need to mount it and copy the file onto disk

	cmd = "sudo mount -o loop,rw,sync BlockStorage.img mountPoint";

	system(cmd.c_str());

	//Now copy the file to disk
	cmd = "sudo mv " + name + " mountPoint";
	cout << "Copied file to the mounted storage\n";

	result = exec(cmd.c_str());

	cout << "Copying result :" << result << endl;

	cmd = "sudo umount BlockStorage.img";
	result = exec(cmd.c_str());
	cout << "unmount :" << result << endl;	

	return "SUCCESS";

   
}

//download object
std::string node::objectDownload(std::string fileName) {

   //Create a file first
	ifstream fs;
	string cmd;
	string result;
	string content;
	string name = fileName + ".txt";
	

	
	cmd = "sudo mount -o loop,rw,sync BlockStorage.img mountPoint";

	system(cmd.c_str());

	//Now copy the file to disk
	cmd = "sudo cp mountPoint/"+name+" .";
	cout << "Copied file to the local storage\n";

	result = exec(cmd.c_str());

	cout << "Copying result :" << result << endl;

	//Reaed the file
	fs.open(name.c_str());
	string str;
	while(std::getline(fs, str)){

		content += str;
	}

	fs.close();

	cmd = "sudo rm " + name;
	result = exec(cmd.c_str());

	cout << "Result of remove: " << result << endl;


	cmd = "sudo umount BlockStorage.img";
	result = exec(cmd.c_str());
	cout << "unmount :" << result << endl;	

	cout << "Content : " << content << endl;

	return content;
}
