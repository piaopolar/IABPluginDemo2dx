//
//  PaymentInterface.mm
//
//  Created by Ruoqian, Chen on 2015/01/14
//
//  reference: http://www.cocos2d-x.org/docs/tutorial/framework/html5/jsb-ios-iap/ios-storekit-integration-in-c++/zh

#include "StdAfx.h"

#import <StoreKit/StoreKit.h>
#include "PaymentInterface.h"
#include "PaymentMgr.h"
#include "UITip.h"
#include "spine/Json.h"

@interface iAPProductsRequestDelegate : NSObject<SKProductsRequestDelegate>
{
	NSArray* m_pArrProducts;
}

- (id) init;
- (void) paymentWithIdentifier: (NSString*)Identifier;
@end


@implementation iAPProductsRequestDelegate
- (id)init
{
	m_pArrProducts = nil;
	return self;
}

-(void)paymentWithIdentifier:(NSString*)Identifier
{
	NSUInteger count = 0;
	if (m_pArrProducts != nil) {
		count = [m_pArrProducts count];
	}
	
	NSLog(@"paymentWithIdentifier %zd [%@]", count, Identifier);
	
	if (count <= 0) {
		return;
	}

	for (SKProduct* product in m_pArrProducts) {
		if ([[product productIdentifier] compare: Identifier] == 0) {
			SKPayment *payment = [SKPayment paymentWithProduct: product];
			[[SKPaymentQueue defaultQueue] addPayment: payment];
			break;
		}
	}
}


- (void)productsRequest:(SKProductsRequest *)request
	 didReceiveResponse:(SKProductsResponse *)response
{
	// release old
	if (m_pArrProducts) {
		[m_pArrProducts release];
	}

	// record new product
	m_pArrProducts = [response.products retain];
	int nCount = [response.products count];
	
	for (int index = 0; index < nCount; index++) {
		SKProduct *skProduct = [response.products objectAtIndex:index];
		
		// check is valid
		bool isValid = true;
		for (NSString *invalidIdentifier in response.invalidProductIdentifiers) {
			NSLog(@"invalidIdentifier:%@", invalidIdentifier);
			if ([skProduct.productIdentifier isEqualToString:invalidIdentifier]) {
				isValid = false;
				break;
			}
		}
		
		if (!isValid) {
			continue;
		}
		
		PAY_ITEMINFO info;
		info.m_strTypeId = [skProduct.productIdentifier UTF8String];
		info.m_strName = [skProduct.localizedTitle UTF8String];
		info.m_strDesc = [skProduct.localizedDescription UTF8String];
		
		// locale price to string
		NSNumberFormatter *formatter = [[NSNumberFormatter alloc] init];
		[formatter setFormatterBehavior:NSNumberFormatterBehavior10_4];
		[formatter setNumberStyle:NSNumberFormatterCurrencyStyle];
		[formatter setLocale:skProduct.priceLocale];
		NSString *priceStr = [formatter stringFromNumber:skProduct.price];
		[formatter release];
		info.m_strPrice = [priceStr UTF8String];
		
		PaymentMgr::GetInstance()->AddItemInfo(info);
	}
	
	PaymentMgr::GetInstance()->NotifyObserver(PAY_EVENT_QUERY_SKU_FIN);
}

- (void)requestDidFinish:(SKRequest *)request
{
	CCLOG("requestDidFinish");
}

- (void)request:(SKRequest *)request didFailWithError:(NSError *)error
{
	NSString* pStrError = error.localizedDescription;
	const char *pszError = pStrError ? [pStrError UTF8String] : "";
	CCLOG("request didFailWithError [%d] [%s]", error.code, pszError);
}

@end


@interface iAPTransactionObserver : NSObject<SKPaymentTransactionObserver>
{
}
@end

@implementation iAPTransactionObserver

- (void)paymentQueue:(SKPaymentQueue *)queue updatedTransactions:(NSArray *)transactions
{
	for (SKPaymentTransaction *transaction in transactions) {
		NSString* transactionIdentifier = transaction.transactionIdentifier;
		NSString* productIdentifier = transaction.payment.productIdentifier;
		NSString* receipt = [transaction.transactionReceipt base64Encoding];
		const char *pszTransactionId = [transactionIdentifier UTF8String];
		const char *pszItemTypeId = [productIdentifier UTF8String];
		const char *pszReceipt = [receipt UTF8String];
		switch (transaction.transactionState) {
			case SKPaymentTransactionStatePurchasing:
				break;
			case SKPaymentTransactionStatePurchased:
				PaymentMgr::GetInstance()->OnPurchased(pszTransactionId, pszItemTypeId, pszReceipt);
				break;
			case SKPaymentTransactionStateFailed:
				{
					const char *pszError = "";
					switch (transaction.error.code) {
						case SKErrorUnknown:
							pszError = "SKErrorUnknown";
							break;
						case SKErrorClientInvalid:
							pszError = "SKErrorClientInvalid";
							break;
						case SKErrorPaymentCancelled:
							pszError = "SKErrorPaymentCancelled";
							break;
						case SKErrorPaymentInvalid:
							pszError = "SKErrorPaymentInvalid";
							break;
						case SKErrorPaymentNotAllowed:
							pszError = "SKErrorPaymentNotAllowed";
							break;
						case SKErrorStoreProductNotAvailable:
							pszError = "SKErrorStoreProductNotAvailable";
							break;
						default:
							pszError = "Error";
							break;
					}
					
					PaymentMgr::GetInstance()->OnFailed(pszTransactionId, pszError);
				}
				break;
			case SKPaymentTransactionStateRestored:
				PaymentMgr::GetInstance()->OnRestore(pszTransactionId, pszItemTypeId, pszReceipt);
				break;
			default:
				break;
		}
	}
}

- (void)paymentQueue:(SKPaymentQueue *)queue removedTransactions:(NSArray *)transactions
{
	for (SKPaymentTransaction *transaction in transactions) {
		NSString* identifier = transaction.payment.productIdentifier;
		const char *pszIdentifier = [identifier UTF8String];
		CCLOG("removedTransactions: %s", pszIdentifier);
	}
}

@end


static iAPProductsRequestDelegate* s_pProductsRequestDelegate = nil;
static iAPTransactionObserver *s_pTransactionObserver = nil;

void PaymentInterface::Init(void)
{
	s_pProductsRequestDelegate = [[iAPProductsRequestDelegate alloc] init];
	s_pTransactionObserver = [[iAPTransactionObserver alloc] init];
	[[SKPaymentQueue defaultQueue] addTransactionObserver: s_pTransactionObserver];
}

void PaymentInterface::ReqItemInfo( const std::vector<std::string>& vecItemTypeId )
{
	NSMutableSet* pSet = [NSMutableSet setWithCapacity: vecItemTypeId.size()];
	for (auto &rProductIdentifier : vecItemTypeId) {
		[pSet addObject: [NSString stringWithUTF8String: rProductIdentifier.c_str()]];
	}

	SKProductsRequest *productsRequest = [[SKProductsRequest alloc] initWithProductIdentifiers: pSet];
	productsRequest.delegate = s_pProductsRequestDelegate;
	[productsRequest start];
}

void PaymentInterface::PayStart( const char *pszItemTypeId, const char *pszExtraVerifyInfo )
{
	[s_pProductsRequestDelegate paymentWithIdentifier: [NSString stringWithUTF8String: pszItemTypeId]];
}

void PaymentInterface::PayEnd( const char *pszItemKey )
{
	NSString* pStrIdentifier = [NSString stringWithUTF8String: pszItemKey];
	NSArray* pArrTransaction = [SKPaymentQueue defaultQueue].transactions;
	
	for (SKPaymentTransaction* pTransaction in pArrTransaction) {
		if ([[pTransaction transactionIdentifier] compare : pStrIdentifier] == 0) {
			[[SKPaymentQueue defaultQueue] finishTransaction: pTransaction];
		}
	}
}
