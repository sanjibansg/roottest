#include <vector>
#include "TObject.h"
#include "TString.h"

class Content {
public:
   Content() : fId(0) {};
   UInt_t fId;
   TString fName;
   const char *GetName() const { return fName; }
   void SetName(const char *name) { fName = name; }
   
   ClassDef(Content,3);
};


template <typename T>
class DataVectorTmplt {
public:
   std::vector<T> fValues; //!

   void Fill(unsigned long seed) {
      T obj;
      for(size_t i = 0; i < seed; ++i) {
         obj.SetName(TString::Format("name%lu_%lu",i,seed));
         obj.fId = i;
         fValues.push_back(obj);
      }
   }

   void Print() {
      for(size_t i = 0; i < fValues.size(); ++i) {
         printf("values: %lu / %lu : %s\n",
                i, fValues.size(), fValues[i].GetName());
      }
   }
};

class DataVectorConcrete {
public:
   std::vector<Content> fValues; //!

   void Fill(unsigned long seed) {
      Content obj;
      for(size_t i = 0; i < seed; ++i) {
         obj.SetName(TString::Format("name%lu_%lu",i,seed));
         obj.fId = i;
         fValues.push_back(obj);
      }
   }

   void Print() {
      for(size_t i = 0; i < fValues.size(); ++i) {
         printf("values: %lu / %lu : %s\n",
                i, fValues.size(), fValues[i].GetName());
      }
   }
};

#ifdef __MAKECINT__
#pragma link C++ class DataVectorTmplt<Content>+;
#pragma link C++ class vector<Content>+;
#endif

typedef DataVectorTmplt<Content> DataVector;

#include "TFile.h"
#include "TClass.h"
#include "TTree.h"

   // This breaks the test on windows.
#ifndef protected
#define protected public
#endif
#include "TVirtualCollectionProxy.h"

void MakeCollection(const char *classname, const char *equiv) 
{
   //   TClass *c = TClass::GetClass("DataVector");
   TClass *c = TClass::GetClass(classname);

   TClass *e = TClass::GetClass(equiv);
   if (!e) {
      printf("Error could not find or create the TClass for \"%s\".\n",equiv);
      return;
   }
   c->CopyCollectionProxy( * TClass::GetClass(equiv)->GetCollectionProxy() );

   // This breaks the test on windows.
   c->GetCollectionProxy()->fClass = c;
}

int execWriteCustomCollection() {
   TFile *file = TFile::Open("coll.root","RECREATE");
   if (!file) return 1;

   MakeCollection("DataVector","vector<Content>");
   MakeCollection("DataVectorConcrete","vector<Content>");
   
   DataVector v;
   v.Fill(3);
   printf("Writing\n");
   v.Print();
   file->WriteObject(&v,"coll");
   TTree *tree = new TTree("T","T");
   tree->Branch("coll.",&v);
   tree->Branch("vec.",&v.fValues);
   tree->Fill();
   tree->Scan();
   file->Write();
   delete file;

   printf("Writing file with just a TTree.\n");
   DataVectorConcrete vc;
   vc.Fill(4);
   file = TFile::Open("tcoll.root","RECREATE");
   if (!file) return 1;
   tree = new TTree("T","T");
   tree->Branch("coll.",&vc);
   tree->Branch("vec.",&vc.fValues);
   tree->Fill();
   file->Write();
   delete file;

   printf("Reading\n");
   DataVector *vp = 0;
   file = TFile::Open("coll.root","READ");
   file->GetObject("coll",vp);
   if (vp) {
      vp->Print();
   }
   printf("Reading TTree\n");
   file->GetObject("T",tree);
   if (tree) {
      DataVector *tvp = 0;
      tree->SetBranchAddress("coll",&tvp);
      tree->GetEntry(0);
      if (tvp) tvp->Print();
   }
   tree->Scan();
   delete file;

   printf("Reading file with just a TTree\n");
   file = TFile::Open("tcoll.root","READ");
   file->GetObject("T",tree);
   if (tree) {
      DataVectorConcrete *tvp = 0;
      tree->SetBranchAddress("coll",&tvp);
      tree->GetEntry(0);
      if (tvp) tvp->Print();
   }
   tree->Scan();
   delete file;
   
   return 0;
}
