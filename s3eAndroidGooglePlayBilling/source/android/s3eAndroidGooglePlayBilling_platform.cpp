/*
 * android-specific implementation of the s3eAndroidGooglePlayBilling extension.
 * Add any platform-specific functionality here.
 */
/*
 * NOTE: This file was originally written by the extension builder, but will not
 * be overwritten (unless --force is specified) and is intended to be modified.
 */
#include "s3eAndroidGooglePlayBilling_internal.h"

#include "s3eEdk.h"
#include "s3eEdk_android.h"
#include <jni.h>
#include "IwDebug.h"

static jobject g_Obj;
static jmethodID g_s3eAndroidGooglePlayBillingStart;
static jmethodID g_s3eAndroidGooglePlayBillingStop;
static jmethodID g_s3eAndroidGooglePlayBillingIsSupported;
static jmethodID g_s3eAndroidGooglePlayBillingRequestPurchase;
static jmethodID g_s3eAndroidGooglePlayBillingRequestProductInformation;
static jmethodID g_s3eAndroidGooglePlayBillingRestoreTransactions;
static jmethodID g_s3eAndroidGooglePlayBillingConsumeItem;

void JNICALL s3e_ANDROIDGOOGLEPLAYBILLING_PURCHASE_CALLBACK( JNIEnv* env,  jobject obj, jint status, jstring errorMsg, jobject purchaseData);
void JNICALL s3e_ANDROIDGOOGLEPLAYBILLING_LIST_PRODUCTS_CALLBACK( JNIEnv* env,  jobject obj, jint status, jstring errorMsg, jobjectArray products);
void JNICALL s3e_ANDROIDGOOGLEPLAYBILLING_RESTORE_CALLBACK( JNIEnv* env,  jobject obj, jint status, jstring errorMsg, jobjectArray purchases);
void JNICALL s3e_ANDROIDGOOGLEPLAYBILLING_CONSUME_CALLBACK( JNIEnv* env,  jobject obj, jint status, jstring errorMsg);

s3eResult s3eAndroidGooglePlayBillingInit_platform()
{
    // Get the environment from the pointer
    JNIEnv* env = s3eEdkJNIGetEnv();
    jobject obj = NULL;
    jmethodID cons = NULL;

    // Get the extension class
    jclass cls = s3eEdkAndroidFindClass("com/ideaworks3d/marmalade/s3eAndroidGooglePlayBilling/s3eAndroidGooglePlayBilling");
    if (!cls)
        goto fail;

    // Get its constructor
    cons = env->GetMethodID(cls, "<init>", "()V");
    if (!cons)
        goto fail;

    // Construct the java class
    obj = env->NewObject(cls, cons);
    if (!obj)
        goto fail;

    // Get all the extension methods
    g_s3eAndroidGooglePlayBillingStart = env->GetMethodID(cls, "s3eAndroidGooglePlayBillingStart", "(Ljava/lang/String;)I");
    if (!g_s3eAndroidGooglePlayBillingStart)
        goto fail;

    g_s3eAndroidGooglePlayBillingStop = env->GetMethodID(cls, "s3eAndroidGooglePlayBillingStop", "()V");
    if (!g_s3eAndroidGooglePlayBillingStop)
        goto fail;

    g_s3eAndroidGooglePlayBillingIsSupported = env->GetMethodID(cls, "s3eAndroidGooglePlayBillingIsSupported", "()I");
    if (!g_s3eAndroidGooglePlayBillingIsSupported)
        goto fail;

    g_s3eAndroidGooglePlayBillingRequestPurchase = env->GetMethodID(cls, "s3eAndroidGooglePlayBillingRequestPurchase", "(Ljava/lang/String;ZLjava/lang/String;)V");
    if (!g_s3eAndroidGooglePlayBillingRequestPurchase)
        goto fail;

    g_s3eAndroidGooglePlayBillingRequestProductInformation = env->GetMethodID(cls, "s3eAndroidGooglePlayBillingRequestProductInformation", "([Ljava/lang/String;[Ljava/lang/String;)V");
    if (!g_s3eAndroidGooglePlayBillingRequestProductInformation)
        goto fail;

    g_s3eAndroidGooglePlayBillingRestoreTransactions = env->GetMethodID(cls, "s3eAndroidGooglePlayBillingRestoreTransactions", "()V");
    if (!g_s3eAndroidGooglePlayBillingRestoreTransactions)
        goto fail;

    g_s3eAndroidGooglePlayBillingConsumeItem = env->GetMethodID(cls, "s3eAndroidGooglePlayBillingConsumeItem", "(Ljava/lang/String;)V");
    if (!g_s3eAndroidGooglePlayBillingConsumeItem)
        goto fail;

	// Non-autogenerated - register the native hooks
    {
        static const JNINativeMethod methods[]=
        {
			{"native_PURCHASE_CALLBACK",			"(ILjava/lang/String;Lcom/ideaworks3d/marmalade/s3eAndroidGooglePlayBilling/s3eAndroidGooglePlayBilling$S3eBillingPurchase;)V",								(void*)s3e_ANDROIDGOOGLEPLAYBILLING_PURCHASE_CALLBACK},		
			{"native_LIST_PRODUCTS_CALLBACK",		"(ILjava/lang/String;[Lcom/ideaworks3d/marmalade/s3eAndroidGooglePlayBilling/s3eAndroidGooglePlayBilling$S3eBillingItemInfo;)V",							(void*)s3e_ANDROIDGOOGLEPLAYBILLING_LIST_PRODUCTS_CALLBACK},
			{"native_RESTORE_CALLBACK",				"(ILjava/lang/String;[Lcom/ideaworks3d/marmalade/s3eAndroidGooglePlayBilling/s3eAndroidGooglePlayBilling$S3eBillingPurchase;)V",							(void*)s3e_ANDROIDGOOGLEPLAYBILLING_RESTORE_CALLBACK},
			{"native_CONSUME_CALLBACK",				"(ILjava/lang/String;)V",																										(void*)s3e_ANDROIDGOOGLEPLAYBILLING_CONSUME_CALLBACK},
        };
        jint ret = env->RegisterNatives(cls, methods, sizeof(methods)/sizeof(methods[0]));
		if (ret)
		{
			IwTrace(ANDROIDGOOGLEPLAYBILLING, ("ANDROIDGOOGLEPLAYBILLING RegisterNatives failed error:%d",ret));
            goto fail;
		}
    }

    IwTrace(ANDROIDGOOGLEPLAYBILLING, ("ANDROIDGOOGLEPLAYBILLING init success"));
    g_Obj = env->NewGlobalRef(obj);
    env->DeleteLocalRef(obj);
    env->DeleteGlobalRef(cls);

    // Add any platform-specific initialisation code here
    return S3E_RESULT_SUCCESS;

fail:
    jthrowable exc = env->ExceptionOccurred();
    if (exc)
    {
        env->ExceptionDescribe();
        env->ExceptionClear();
        IwTrace(s3eAndroidGooglePlayBilling, ("s3eAndroidGooglePlayBilling: One or more java methods could not be found"));
    }
    return S3E_RESULT_ERROR;

}

/** Some helper functions to handle potentially large arrays of strings */

struct s3eAndroidJNIFrame
{
    JNIEnv* env;
	
    s3eAndroidJNIFrame(JNIEnv* env,jint capacity)
    :env(env)
    {
        env->PushLocalFrame(capacity);
    }
    ~s3eAndroidJNIFrame()
    {
        env->PopLocalFrame(NULL);
    }
    JNIEnv* operator->()
    {
        return env;
    }
    JNIEnv* operator()()
    {
        return env;
    }
};

static jobjectArray makeStringArray(const char** strings,int n)
{
    //the parent holds an s3eAndroidJNIFrame
    JNIEnv* env=s3eEdkJNIGetEnv();
    jobjectArray j_strings=env->NewObjectArray(n,env->FindClass("java/lang/String"),NULL);
    for(int i=0;i<n;++i)
    {
        env->SetObjectArrayElement(j_strings,i,env->NewStringUTF(strings[i]));
    }
    return j_strings;
}

void s3eAndroidGooglePlayBillingTerminate_platform()
{
    // Add any platform-specific termination code here
}

void s3eAndroidGooglePlayBillingStart_platform(const char *base64Key)
{
    JNIEnv* env = s3eEdkJNIGetEnv();
    jstring base64Key_jstr = env->NewStringUTF(base64Key);
    env->CallIntMethod(g_Obj, g_s3eAndroidGooglePlayBillingStart, base64Key_jstr);
}

void s3eAndroidGooglePlayBillingStop_platform()
{
    JNIEnv* env = s3eEdkJNIGetEnv();
    env->CallVoidMethod(g_Obj, g_s3eAndroidGooglePlayBillingStop);
}

s3eResult s3eAndroidGooglePlayBillingIsSupported_platform()
{
    JNIEnv* env = s3eEdkJNIGetEnv();
    return (s3eResult)env->CallIntMethod(g_Obj, g_s3eAndroidGooglePlayBillingIsSupported);
}

void s3eAndroidGooglePlayBillingRequestPurchase_platform(const char* productID, bool inApp, const char* develperPayLoad)
{
    JNIEnv* env = s3eEdkJNIGetEnv();
    jstring productID_jstr = env->NewStringUTF(productID);
    jstring develperPayLoad_jstr = env->NewStringUTF(develperPayLoad);
    env->CallVoidMethod(g_Obj, g_s3eAndroidGooglePlayBillingRequestPurchase, productID_jstr, inApp, develperPayLoad_jstr);
}

void s3eAndroidGooglePlayBillingRequestProductInformation_platform(const char** inAppProducts,int numInAppProducts, const char** subProducts, int numSubProducts)
{
 	s3eAndroidJNIFrame env(s3eEdkJNIGetEnv(),numInAppProducts + numSubProducts); // this local frame is needed to make sure there is enough stack to hold the strings
 	jobjectArray inAppArray = makeStringArray( inAppProducts, numInAppProducts );
	jobjectArray subsArray = makeStringArray( subProducts, numSubProducts );	
	env->CallVoidMethod(g_Obj, g_s3eAndroidGooglePlayBillingRequestProductInformation, inAppArray ,subsArray);
}

void s3eAndroidGooglePlayBillingRestoreTransactions_platform()
{
    JNIEnv* env = s3eEdkJNIGetEnv();
    env->CallVoidMethod(g_Obj, g_s3eAndroidGooglePlayBillingRestoreTransactions);
}

void s3eAndroidGooglePlayBillingConsumeItem_platform(const char* purchaseToken)
{
    JNIEnv* env = s3eEdkJNIGetEnv();
    jstring purchaseToken_jstr = env->NewStringUTF(purchaseToken);
    env->CallVoidMethod(g_Obj, g_s3eAndroidGooglePlayBillingConsumeItem, purchaseToken_jstr);
}

static char* JStringFieldToChar(jobject obj,jfieldID field)
{
    JNIEnv* env=s3eEdkJNIGetEnv();
    jstring str=(jstring)env->GetObjectField(obj,field);
    if(!str)
    {
        return NULL;
    }
    jsize len=env->GetStringUTFLength(str);
    const char* utf=env->GetStringUTFChars(str,NULL);
    char* retval=new char[len+1];
    memcpy(retval,utf,len+1); //faster strcpy
    env->ReleaseStringUTFChars(str,utf);
    return retval;
}

static char* JStringToChar( JNIEnv* env, jstring str )
{
	if( str )
	{
		jsize len = env->GetStringUTFLength( str );

		const char* utf=env->GetStringUTFChars( str, NULL );
		char* retval = new char[ len + 1 ];
		memcpy( retval, utf, len + 1 );
		env->ReleaseStringUTFChars( str, utf );
		
		return retval;
	}

	return NULL;
}

void ConvertPurchase(JNIEnv* env,  jobject purchaseData, s3eAndroidGooglePlayBillingPurchase *p)
{
    jclass j_orderclass=env->FindClass("com/ideaworks3d/marmalade/s3eAndroidGooglePlayBilling/s3eAndroidGooglePlayBilling$S3eBillingPurchase");
    jfieldID jf_m_OrderID  			= env->GetFieldID(j_orderclass,"m_OrderID"  		,"Ljava/lang/String;");	
    jfieldID jf_m_PackageID  		= env->GetFieldID(j_orderclass,"m_PackageID"  		,"Ljava/lang/String;");	
    jfieldID jf_m_ProductId  		= env->GetFieldID(j_orderclass,"m_ProductId"  		,"Ljava/lang/String;");	
    jfieldID jf_m_PurchaseTime      = env->GetFieldID(j_orderclass,"m_PurchaseTime" 	,"J"				 );
    jfieldID jf_m_PurchaseState     = env->GetFieldID(j_orderclass,"m_PurchaseState"    ,"I"				 );
    jfieldID jf_m_PurchaseToken     = env->GetFieldID(j_orderclass,"m_PurchaseToken"    ,"Ljava/lang/String;");
    jfieldID jf_m_DeveloperPayload  = env->GetFieldID(j_orderclass,"m_DeveloperPayload" ,"Ljava/lang/String;");
    jfieldID jf_m_JSON		        = env->GetFieldID(j_orderclass,"m_JSON"    			,"Ljava/lang/String;");
    jfieldID jf_m_Signature		    = env->GetFieldID(j_orderclass,"m_Signature"   		,"Ljava/lang/String;");
	
	p->m_OrderID 				= JStringFieldToChar(purchaseData,jf_m_OrderID);
	p->m_PackageID 				= JStringFieldToChar(purchaseData,jf_m_PackageID);
	p->m_ProductId				= JStringFieldToChar(purchaseData,jf_m_ProductId);
	p->m_PurchaseTime			= env->GetLongField(purchaseData,jf_m_PurchaseTime);
	p->m_PurchaseState			= env->GetIntField(purchaseData,jf_m_PurchaseState);
	p->m_PurchaseToken			= JStringFieldToChar(purchaseData,jf_m_PurchaseToken);
	p->m_DeveloperPayload		= JStringFieldToChar(purchaseData,jf_m_DeveloperPayload);
	p->m_JSON					= JStringFieldToChar(purchaseData,jf_m_JSON);
	p->m_Signature				= JStringFieldToChar(purchaseData,jf_m_Signature);
}

void DeletePurchase(s3eAndroidGooglePlayBillingPurchase *p)
{
	delete []p->m_OrderID;
	delete []p->m_PackageID;
	delete []p->m_ProductId;
	delete []p->m_PurchaseToken;
	delete []p->m_DeveloperPayload;
	delete []p->m_JSON;
	delete []p->m_Signature;
}

static void s3eAGC_DeallocatePurchase(uint32 extID, int32 notification, void *systemData, void *instance, int32 returnCode, void *completeData)
{
	s3eAndroidGooglePlayBillingPurchaseResponse *pr = (s3eAndroidGooglePlayBillingPurchaseResponse*)systemData;
	if (pr->m_PurchaseDetails)
		DeletePurchase(pr->m_PurchaseDetails);
	if (pr->m_ErrorMsg)
		delete []pr->m_ErrorMsg;
	delete pr;
}

// Our Native callbacks - these are called from Java

void JNICALL s3e_ANDROIDGOOGLEPLAYBILLING_PURCHASE_CALLBACK( JNIEnv* env,  jobject obj, jint status, jstring errorMsg, jobject purchaseData)
{
	s3eAndroidGooglePlayBillingPurchaseResponse *pr = new s3eAndroidGooglePlayBillingPurchaseResponse;
	pr->m_ErrorMsg = JStringToChar(env, errorMsg);
	pr->m_Status = (s3eAndroidGooglePlayBillingResult)status;
	
	if (purchaseData)
	{
		pr->m_PurchaseDetails = new s3eAndroidGooglePlayBillingPurchase;
		ConvertPurchase(env, purchaseData, pr->m_PurchaseDetails);
	}
	else
		pr->m_PurchaseDetails = NULL;
	
	s3eEdkCallbacksEnqueue(S3E_EXT_ANDROIDGOOGLEPLAYBILLING_HASH,S3E_ANDROIDGOOGLEPLAYBILLING_PURCHASE_CALLBACK,pr,0,NULL,false,s3eAGC_DeallocatePurchase,pr);
}

void ConvertSku(JNIEnv* env,  jobject itemData, s3eAndroidGooglePlayBillingItemInfo *p)
{
    jclass j_orderclass=env->FindClass("com/ideaworks3d/marmalade/s3eAndroidGooglePlayBilling/s3eAndroidGooglePlayBilling$S3eBillingItemInfo");
    jfieldID jf_m_ProductID			= env->GetFieldID(j_orderclass,"m_ProductID"  		,"Ljava/lang/String;");	
    jfieldID jf_m_Type		  		= env->GetFieldID(j_orderclass,"m_Type"  			,"Ljava/lang/String;");	
    jfieldID jf_m_Price		  		= env->GetFieldID(j_orderclass,"m_Price"  			,"Ljava/lang/String;");	
    jfieldID jf_m_Title      		= env->GetFieldID(j_orderclass,"m_Title" 			,"Ljava/lang/String;");
    jfieldID jf_m_Description     	= env->GetFieldID(j_orderclass,"m_Description"    	,"Ljava/lang/String;");

	p->m_ProductID 				= JStringFieldToChar(itemData,jf_m_ProductID);
	p->m_Type	 				= JStringFieldToChar(itemData,jf_m_Type);
	p->m_Price					= JStringFieldToChar(itemData,jf_m_Price);
	p->m_Title					= JStringFieldToChar(itemData,jf_m_Title);
	p->m_Description			= JStringFieldToChar(itemData,jf_m_Description);
}

void DeleteItemData(s3eAndroidGooglePlayBillingItemInfo *p)
{
	delete []p->m_ProductID;
	delete []p->m_Type;
	delete []p->m_Price;
	delete []p->m_Title;
	delete []p->m_Description;
}

static void s3eAGC_DeallocateItemList(uint32 extID, int32 notification, void *systemData, void *instance, int32 returnCode, void *completeData)
{
	s3eAndroidGooglePlayBillingSkuResponse *item = (s3eAndroidGooglePlayBillingSkuResponse*)systemData;
	for (int i=0;i<item->m_NumProducts;i++)
		DeleteItemData(&item->m_Products[i]);
	if (item->m_Products)
		delete []item->m_Products;
	if (item->m_ErrorMsg)
		delete []item->m_ErrorMsg;
	delete item;
}

void JNICALL s3e_ANDROIDGOOGLEPLAYBILLING_LIST_PRODUCTS_CALLBACK( JNIEnv* env,  jobject obj, jint status, jstring errorMsg, jobjectArray products)
{
	s3eAndroidGooglePlayBillingSkuResponse *sr = new s3eAndroidGooglePlayBillingSkuResponse;
	sr->m_ErrorMsg = JStringToChar(env, errorMsg);
	sr->m_Status = (s3eAndroidGooglePlayBillingResult)status;
	
	if ((products) && (env->GetArrayLength(products)))
	{
		sr->m_NumProducts = env->GetArrayLength(products);
		sr->m_Products = new s3eAndroidGooglePlayBillingItemInfo[sr->m_NumProducts];
		for (int i=0;i<sr->m_NumProducts;i++)
		{
			jobject j_item=env->GetObjectArrayElement(products,i);
			ConvertSku(env,j_item,&sr->m_Products[i]);
		}
	}
	else
	{
		sr->m_NumProducts = 0;
		sr->m_Products = NULL;
	}
	
	s3eEdkCallbacksEnqueue(S3E_EXT_ANDROIDGOOGLEPLAYBILLING_HASH,S3E_ANDROIDGOOGLEPLAYBILLING_LIST_PRODUCTS_CALLBACK,sr,0,NULL,false,s3eAGC_DeallocateItemList,sr);
}

static void s3eAGC_DeallocatePurchases(uint32 extID, int32 notification, void *systemData, void *instance, int32 returnCode, void *completeData)
{
	s3eAndroidGooglePlayBillingRestoreResponse *rr = (s3eAndroidGooglePlayBillingRestoreResponse*)systemData;
	for (int i=0;i<rr->m_NumPurchases;i++)
		DeletePurchase(&rr->m_Purchases[i]);
	if (rr->m_Purchases)
		delete []rr->m_Purchases;
	if (rr->m_ErrorMsg)
		delete []rr->m_ErrorMsg;
	delete rr;
}

void JNICALL s3e_ANDROIDGOOGLEPLAYBILLING_RESTORE_CALLBACK( JNIEnv* env,  jobject obj, jint status, jstring errorMsg, jobjectArray purchases)
{
	s3eAndroidGooglePlayBillingRestoreResponse *rr = new s3eAndroidGooglePlayBillingRestoreResponse;
	rr->m_ErrorMsg = JStringToChar(env, errorMsg);
	rr->m_Status = (s3eAndroidGooglePlayBillingResult)status;
	
	if ((purchases) && (env->GetArrayLength(purchases)))
	{
		rr->m_NumPurchases = env->GetArrayLength(purchases);
		rr->m_Purchases = new s3eAndroidGooglePlayBillingPurchase[rr->m_NumPurchases];
		for (int i=0;i<rr->m_NumPurchases;i++)
		{
			jobject j_purchase=env->GetObjectArrayElement(purchases,i);
			ConvertPurchase(env,j_purchase,&rr->m_Purchases[i]);
		}
	}
	else
	{
		rr->m_NumPurchases = 0;
		rr->m_Purchases = NULL;
	}
	
	s3eEdkCallbacksEnqueue(S3E_EXT_ANDROIDGOOGLEPLAYBILLING_HASH,S3E_ANDROIDGOOGLEPLAYBILLING_RESTORE_CALLBACK,rr,0,NULL,false,s3eAGC_DeallocatePurchases,rr);
}

static void s3eAGC_DeallocateConsume(uint32 extID, int32 notification, void *systemData, void *instance, int32 returnCode, void *completeData)
{
	s3eAndroidGooglePlayBillingConsumeResponse *cr = (s3eAndroidGooglePlayBillingConsumeResponse*)systemData;
	if (cr->m_ErrorMsg)
		delete []cr->m_ErrorMsg;
	delete cr;
}

void JNICALL s3e_ANDROIDGOOGLEPLAYBILLING_CONSUME_CALLBACK( JNIEnv* env,  jobject obj, jint status, jstring errorMsg)
{
	s3eAndroidGooglePlayBillingConsumeResponse *cr = new s3eAndroidGooglePlayBillingConsumeResponse;
	cr->m_ErrorMsg = JStringToChar(env, errorMsg);
	cr->m_Status = (s3eAndroidGooglePlayBillingResult)status;
	
	s3eEdkCallbacksEnqueue(S3E_EXT_ANDROIDGOOGLEPLAYBILLING_HASH,S3E_ANDROIDGOOGLEPLAYBILLING_CONSUME_CALLBACK,cr,0,NULL,false,s3eAGC_DeallocateConsume,cr);
}
