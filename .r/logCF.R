i<- complex(real=0,imag=1);

psi<-function(u,sigma,nu,theta)
{
  temp = -1/nu*log( 1 - i*u*theta*nu + sigma^2*nu*u^2/2 );
  return(temp);  
}

phi<-function(u,T,y,kappa , eta, lambda)
{  
  gamma = sqrt(kappa^2 - 2*lambda^2*i*u);
  
  temp1 = exp(kappa^2*eta*T/(lambda^2));
  temp2 = (cosh(gamma*T/2) + kappa/gamma * sinh(gamma*T/2));
  temp3 = 2*kappa*eta/lambda^2;
  
  A = temp1/(temp2^temp3);
  B = 2*i*u/(kappa + gamma*cosh(gamma*T/2)/sinh(gamma*T/2));

  return(A*exp(B*y));
}

VGSA_CF<-function(S,u,sigma,nu,theta, kappa, eta, lambda,r,q,T)
{  
  temp1 = exp(i*u*(log(S) + (r-q)*T));
  print(temp1);
  temp2 = phi(-i*psi(u,sigma,nu,theta),T,1/nu,kappa,eta,lambda);
  print(temp2);
  temp3 = phi(-i*psi(-i,sigma,nu,theta),T,1/nu,kappa,eta,lambda)^(i*u);

  return(temp1 * temp2/temp3);
  
}


print ( VGSA_CF(100,1,.2,.2,-.1,2.5,3.0,1,0,0,.5) );
#print ( VGSA_CF(100,.5,.2,.2,-.1,2.5,3.0,1,0,0,.5) );
#print ( VGSA_CF(100,.2,.2,.2,-.1,2.5,3.0,1,0,0,.5) );
