#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/IRBuilder.h"
#if LLVM_VERSION_MAJOR >= 4
#include "llvm/Bitcode/BitcodeWriter.h"
#else
#include "llvm/Bitcode/ReaderWriter.h"
#endif
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/raw_ostream.h"


static llvm::Value* create_global_ptr(llvm::IRBuilder<>& builder, llvm::Module* module, llvm::Constant* c)
{
    llvm::GlobalVariable* gv = new llvm::GlobalVariable(*module,
        c->getType(), true, llvm::GlobalValue::PrivateLinkage, c);
#if LLVM_VERSION_MAJOR >= 4
    gv->setUnnamedAddr(llvm::GlobalValue::UnnamedAddr::Local);
#else
    gv->setUnnamedAddr(true);
#endif
    llvm::Value* zero = builder.getInt32(0);
    llvm::Value* args[] = {zero, zero};
    llvm::Value* ptr = builder.CreateInBoundsGEP(gv, args, "");
    return ptr;
}

void create_helloworld()
{
#if LLVM_VERSION_MAJOR >= 4
    llvm::LLVMContext context;
#else
    llvm::LLVMContext& context = llvm::getGlobalContext();
#endif
    llvm::IRBuilder<> builder(context);

    // module
    llvm::Module* module = new llvm::Module("top", context);

    // create main function
    llvm::Function* mainfunc = llvm::Function::Create(
        llvm::FunctionType::get(builder.getVoidTy(), false),
        llvm::Function::ExternalLinkage, "main", module);
    mainfunc->setCallingConv(llvm::CallingConv::X86_StdCall);

    llvm::BasicBlock* entry = llvm::BasicBlock::Create(context, "entrypoint", mainfunc);
    builder.SetInsertPoint(entry);

    // declare MessageBox
    llvm::Function* messagebox_func;
    {
        // int32 MessageBoxW(void*, const wchar_t*, const wchar_t*, uint32);
        std::vector<llvm::Type*> args;
        args.push_back(builder.getInt8PtrTy());
        args.push_back(builder.getInt16Ty()->getPointerTo());
        args.push_back(builder.getInt16Ty()->getPointerTo());
        args.push_back(builder.getInt32Ty());
        llvm::FunctionType* messagebox_type = llvm::FunctionType::get(builder.getInt32Ty(), args, false);
        messagebox_func = llvm::Function::Create(
            messagebox_type, llvm::Function::ExternalLinkage, "MessageBoxW", module);
        messagebox_func->setCallingConv(llvm::CallingConv::X86_StdCall);
    }

    // call MessageBox(0, "Hello World", "caption", 0);
    {
        llvm::Constant* a1 = llvm::ConstantDataArray::get(context,
            llvm::ArrayRef<uint16_t>((uint16_t*)L"Hello World\0", 12));
        llvm::Constant* a2 = llvm::ConstantDataArray::get(context,
            llvm::ArrayRef<uint16_t>((uint16_t*)L"caption\0", 8));

        std::vector<llvm::Value*> args;
        args.push_back(llvm::ConstantPointerNull::getNullValue(builder.getInt8PtrTy()));
        args.push_back(create_global_ptr(builder, module, a1));
        args.push_back(create_global_ptr(builder, module, a2));
        args.push_back(builder.getInt32(0));// MB_OK
        llvm::CallInst* inst = builder.CreateCall(messagebox_func, args);
        inst->setCallingConv(llvm::CallingConv::X86_StdCall);
    }

    // declare ExitProcess
    llvm::Function* exitprocess_func;
    {
        // void ExitProcess(uint32)
        llvm::FunctionType* exitprocess_type = llvm::FunctionType::get(
            builder.getVoidTy(), llvm::ArrayRef<llvm::Type*>{builder.getInt32Ty()}, false);
        exitprocess_func = llvm::Function::Create(
            exitprocess_type, llvm::Function::ExternalLinkage, "ExitProcess", module);
        exitprocess_func->setCallingConv(llvm::CallingConv::X86_StdCall);
    }
    // call ExitProcess(0)
    {
        llvm::CallInst* inst = builder.CreateCall(exitprocess_func, builder.getInt32(0));
        inst->setCallingConv(llvm::CallingConv::X86_StdCall);
    }

    // return;
    builder.CreateRetVoid();

    // dump IR code for debug
    //#if !defined(NDEBUG)
    module->dump();
    //#endif

    // write bitcode
#if LLVM_VERSION_MAJOR >= 4
    std::error_code ec;
    llvm::raw_fd_ostream os("helloworld.bc", ec, llvm::sys::fs::OpenFlags::F_None);
#else
    std::string error_info;
    llvm::raw_fd_ostream os("helloworld.bc", error_info, llvm::sys::fs::OpenFlags::F_None);
#endif
    llvm::WriteBitcodeToFile(module, os);
    os.close();
}

int main() {
  create_helloworld();
}
